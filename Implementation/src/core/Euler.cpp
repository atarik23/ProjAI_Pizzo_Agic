#include "Euler.h"
#include <algorithm>
#include <stdexcept>
#include <vector>

/*
  We convert Graph edges into an explicit edge list for traversal:
  Each undirected edge becomes 2 directed edges:
    u -> v (idx a)
    v -> u (idx b)
  Both share the same "usedId" (one per undirected instance)
*/
struct DirectedEdge {
    int to;
    int usedId; // marks whether the undirected edge instance is already used
};

std::vector<int> Euler::eulerCircuit(const Graph& g, int startVertex) {
    const int n = g.vertexCount();
    if (startVertex < 0 || startVertex >= n) {
        throw std::runtime_error("Euler::eulerCircuit: startVertex out of range");
    }

    // validation: all degrees must be even
    for (int v = 0; v < n; ++v) {
        if (g.degree(v) % 2 != 0) {
            throw std::runtime_error("Euler::eulerCircuit: graph is not Eulerian (odd degree exists)");
        }
    }

    // Build directed adjacency for traversal
    std::vector<std::vector<DirectedEdge>> adj(n);
    adj.assign(n, {});

    // There are g.edgeCount() undirected edges, each becomes a "usedId"
    std::vector<char> used(g.edgeCount(), 0);

    // Convert Graph adjacency into directed edges.
    for (int u = 0; u < n; ++u) {
        for (const auto& e : g.adj(u)) {
            adj[u].push_back(DirectedEdge{ e.to, e.id });
        }
    }

    // Iterators to avoid scanning from start each time
    std::vector<size_t> it(n, 0);

    std::vector<int> stack;
    std::vector<int> circuit;
    stack.push_back(startVertex);

    while (!stack.empty()) {
        int v = stack.back();

        // Move iterator until we find an unused edge
        while (it[v] < adj[v].size() && used[adj[v][it[v]].usedId]) {
            it[v]++;
        }

        if (it[v] == adj[v].size()) {
            // no more edges from v, then add to circuit
            circuit.push_back(v);
            stack.pop_back();
        }
        else {
            // follow unused edge
            DirectedEdge de = adj[v][it[v]];
            used[de.usedId] = 1; // consumes the undirected edge for both directions
            stack.push_back(de.to);
        }
    }

    // circuit is built in reverse order
    std::reverse(circuit.begin(), circuit.end());

    // sanity check: should use exactly all edges
    // Euler circuit length should be E + 1
    if ((int)circuit.size() != g.edgeCount() + 1) {
        throw std::runtime_error("Euler::eulerCircuit: circuit length mismatch (graph may be disconnected)");
    }

    return circuit;
}
