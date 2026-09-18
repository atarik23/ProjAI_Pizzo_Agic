#pragma once
#include "Graph.h"
#include <vector>

/*
  Dijkstra result:
  - dist[v] = shortest distance from source to v
  - parentV[v] = previous vertex on shortest path
  - parentEdgeId[v] = which edge id was used to reach v from parentV[v]
*/
struct DijkstraResult {
	static constexpr int INF = 1'000'000'000;

	std::vector<int> dist;
	std::vector<int> parentV;
	std::vector<int> parentEdgeId;
};

class Dijkstra {
public:
	// Runs Dijkstra from source on an undirected weighted graph
	// Throws if source is out of range
	static DijkstraResult run(const Graph& g, int source);

	// Reconstructs the vertex path source to target using parentV
	// Returns empty vector if target unreachable
	static std::vector<int> reconstructVertexPath(const DijkstraResult& r, int source, int target);
};
