#pragma once
#include "Graph.h"
#include "Route.h"

#include <vector>

struct CPPResult {
	int originalCost = 0;
	int addedCost = 0;
	int totalCost = 0;
	std::vector<int> eulerCircuit; // vertex sequence
	Route route; // edge-by-edge traversal
};

class CPP {
public:
	// Solves undirected Chinese Postman for connected graphs.
	// Returns Euler circuit and costs.
	static CPPResult solve(const Graph& g, int startVertex);
};
