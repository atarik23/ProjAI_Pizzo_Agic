#include "CPP.h"
#include "Dijkstra.h"
#include "Matching.h"
#include "Euler.h"
#include "Route.h"

#include <algorithm>
#include <stdexcept>

static int sumOriginalEdgeCosts(const Graph& g) {
    // Graph stores each undirected edge once in edgeCount()
    // But we don't store an explicit list of edges; we can compute total by summing adjacency and dividing by 2
    long long s = 0;
    for (int u = 0; u < g.vertexCount(); ++u) {
        for (const auto& e : g.adj(u)) s += e.cost;
    }
    return (int)(s / 2);
}

static std::vector<int> findOddVertices(const Graph& g) {
    std::vector<int> odd;
    for (int v = 0; v < g.vertexCount(); ++v) {
        if (g.degree(v) % 2 == 1) odd.push_back(v);
    }
    return odd;
}

// Adds edges along the shortest path from a->b
// We add edges between consecutive vertices in the path with the same cost as original shortest path edges
// To do this, we need to find the edge cost between consecutive vertices
// Since the graph may have multiple edges, we take the minimum cost edge between them
static int addPathAsDuplicateEdges(Graph& augmented, const Graph& original, const std::vector<int>& path) {
    int added = 0;
    for (size_t i = 1; i < path.size(); ++i) {
        int u = path[i - 1];
        int v = path[i];

        int bestCost = DijkstraResult::INF;
        for (const auto& e : original.adj(u)) {
            if (e.to == v) bestCost = std::min(bestCost, e.cost);
        }
        if (bestCost == DijkstraResult::INF) {
            throw std::runtime_error("CPP: internal error: path edge not found in original graph");
        }

        augmented.addEdge(u, v, bestCost);
        added += bestCost;
    }
    return added;
}

static void buildRoute(CPPResult& result, const Graph& original) {
    result.route.steps.clear();
    for (size_t i = 1; i < result.eulerCircuit.size(); ++i) {
        const int u = result.eulerCircuit[i - 1];
        const int v = result.eulerCircuit[i];
        result.route.steps.push_back(RouteStep{u, v, original.minEdgeCost(u, v)});
    }
}

CPPResult CPP::solve(const Graph& g, int startVertex) {
    if (g.vertexCount() == 0) throw std::runtime_error("CPP::solve: empty graph");

    CPPResult res;
    res.originalCost = sumOriginalEdgeCosts(g);

    std::vector<int> odd = findOddVertices(g);
    if (odd.empty()) {
        // Already Eulerian
        res.addedCost = 0;
        res.totalCost = res.originalCost;
        res.eulerCircuit = Euler::eulerCircuit(g, startVertex);
        buildRoute(res, g);
        return res;
    }

    // Build odd-vertex distance matrix and store Dijkstra results for reconstruction
    const int k = (int)odd.size();
    std::vector<std::vector<int>> dist(k, std::vector<int>(k, DijkstraResult::INF));
    std::vector<DijkstraResult> dijk(k);

    for (int i = 0; i < k; ++i) {
        dijk[i] = Dijkstra::run(g, odd[i]);
        for (int j = 0; j < k; ++j) dist[i][j] = dijk[i].dist[odd[j]];
    }

    // Minimum perfect matching
    MatchingResult mr = Matching::minPerfectMatching(odd, dist);
    res.addedCost = mr.totalCost;

    // Create augmented graph: start with original edges
    Graph aug = g;

    // Duplicate shortest paths for each matched pair
    int addedCheck = 0;
    for (const auto& p : mr.pairs) {
        int a = odd[p.first];
        int b = odd[p.second];

        std::vector<int> path = Dijkstra::reconstructVertexPath(dijk[p.first], a, b);
        if (path.empty()) {
            throw std::runtime_error("CPP: no path between matched odd vertices (graph disconnected?)");
        }

        addedCheck += addPathAsDuplicateEdges(aug, g, path);
    }

    if (addedCheck != res.addedCost) {
        throw std::runtime_error("CPP: duplicated-path cost does not match the perfect matching cost");
    }

    res.totalCost = res.originalCost + res.addedCost;

    // Euler circuit on augmented graph
    res.eulerCircuit = Euler::eulerCircuit(aug, startVertex);

    buildRoute(res, g);

    return res;
}
