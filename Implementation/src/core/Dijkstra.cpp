#include "Dijkstra.h"
#include <queue>
#include <stdexcept>
#include <algorithm>

DijkstraResult Dijkstra::run(const Graph& g, int source) {
    const int n = g.vertexCount();
    if (source < 0 || source >= n) {
        throw std::runtime_error("Dijkstra::run: source out of range");
    }

    DijkstraResult r;
    r.dist.assign(n, DijkstraResult::INF);
    r.parentV.assign(n, -1);
    r.parentEdgeId.assign(n, -1);

    // (dist, vertex)
    using Item = std::pair<int, int>;
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> pq;

    r.dist[source] = 0;
    pq.push({ 0, source });

    while (!pq.empty()) {
        const int d = pq.top().first;
        const int u = pq.top().second;
        pq.pop();

        // If this is an outdated entry, skip
        if (d != r.dist[u]) continue;

        for (const auto& e : g.adj(u)) {
            const int v = e.to;
            const int nd = d + e.cost;
            if (nd < r.dist[v]) {
                r.dist[v] = nd;
                r.parentV[v] = u;
                r.parentEdgeId[v] = e.id;
                pq.push({ nd, v });
            }
        }
    }

    return r;
}

std::vector<int> Dijkstra::reconstructVertexPath(const DijkstraResult& r, int source, int target) {
    if (target < 0 || target >= (int)r.dist.size()) return {};
    if (r.dist[target] == DijkstraResult::INF) return {};

    std::vector<int> path;
    int cur = target;
    while (cur != -1) {
        path.push_back(cur);
        if (cur == source) break;
        cur = r.parentV[cur];
    }

    if (path.back() != source) return {}; // source not reached

    std::reverse(path.begin(), path.end());
    return path;
}
