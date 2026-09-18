#include "Graph.h"
#include <stdexcept>

void Graph::init(int nVertices) {
    if (nVertices <= 0) {
        throw std::runtime_error("Graph::init: nVertices must be > 0");
    }
    _n = nVertices;
    _m = 0;
    edges.assign(_n, {});
}

void Graph::addEdge(int u, int v, int cost) {
    if (u < 0 || u >= _n || v < 0 || v >= _n) {
        throw std::runtime_error("Graph::addEdge: vertex index out of range");
    }
    if (cost <= 0) {
        throw std::runtime_error("Graph::addEdge: cost must be > 0");
    }

    const int edgeId = _m; // one ID for the undirected edge
    edges[u].push_back(Edge{ v, cost, edgeId });
    edges[v].push_back(Edge{ u, cost, edgeId });
    _m++;
}

int Graph::minEdgeCost(int u, int v) const {
    if (u < 0 || u >= _n || v < 0 || v >= _n) {
        throw std::runtime_error("Graph::minEdgeCost: vertex index out of range");
    }

    int best = 1'000'000'000;
    for (const auto& e : edges[u]) {
        if (e.to == v) {
            if (e.cost < best) best = e.cost;
        }
    }

    if (best == 1'000'000'000) {
        throw std::runtime_error("Graph::minEdgeCost: no edge exists between the given vertices");
    }
    return best;
}
