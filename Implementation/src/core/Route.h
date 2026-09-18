#pragma once
#include <vector>
#include <string>

struct RouteStep {
	int from = -1;
	int to = -1;
	int cost = 0;
};

struct Route {
	std::vector<RouteStep> steps;

	long long totalCost() const;

	// Utility: convert to a human-readable string (labels optional)
	std::string toString(const std::vector<std::string>* labels = nullptr) const;
};
