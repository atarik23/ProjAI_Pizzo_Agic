#pragma once
#include "Graph.h"
#include <vector>

/*
  Builds an Euler circuit in an undirected multigraph using Hierholzer.

  Input: adjacency list with undirected edges (Graph)
  Output: a sequence of vertices representing the Euler circuit walk.
          For an Euler circuit, route.size() == number_of_edge_traversals + 1

  NOTE: The input graph must be connected and all degrees even.
*/
class Euler {
public:
    static std::vector<int> eulerCircuit(const Graph& g, int startVertex);
};
