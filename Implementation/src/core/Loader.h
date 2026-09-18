#pragma once
#include "Graph.h"
#include <string>
#include <vector>

// Data returned from loading a neighborhood file.
struct LoadedNeighborhood {
	Graph graph;
	std::vector<std::string> nodes;
};

class Loader {
public:
	// Loads from a file like res/neighborhood1.txt
	// Throws std::runtime_error if file or format is invalid.
	static LoadedNeighborhood loadNeighborhood(const std::string& filePath);

	// Loads directly from a text buffer (useful for embedded resources / <Texts>)
	static LoadedNeighborhood loadNeighborhoodFromString(const std::string& text, const std::string& virtualName = "<res>");
};
