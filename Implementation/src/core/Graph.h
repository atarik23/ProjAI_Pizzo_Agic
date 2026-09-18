#pragma once
#include <vector>

class Graph {
public:
    struct Edge {
        int to = -1;
        int cost = 0;
        int id = -1;   // unique ID for each undirected edge instance
    };

    Graph() = default;

    // Create graph with N vertices
    void init(int nVertices);

    int vertexCount() const { return _n; }
    int edgeCount() const { return _m; }

    // Add an undirected edge (u <-> v) with cost
    void addEdge(int u, int v, int cost);

    // Read-only adjacency list
    const std::vector<Edge>& adj(int v) const { return edges[v]; }

    // Degree in an undirected multigraph = adjacency size
    int degree(int v) const { return (int)edges[v].size(); }

    // Returns the minimum cost of any edge between u and v
    // Throws if u-v are not directly connected
    int minEdgeCost(int u, int v) const;

    std::vector<std::vector<Edge>> getEdges() const {
        return edges;
    };
private:
    int _n = 0;
    int _m = 0; // number of undirected edges
    std::vector<std::vector<Edge>> edges;
};
