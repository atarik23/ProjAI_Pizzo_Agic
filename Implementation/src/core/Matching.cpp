#include "Matching.h"
#include <stdexcept>
#include <limits>

static void recMatch(const std::vector<std::vector<int>>& dist,
    std::vector<bool>& used,
    int currentCost,
    std::vector<std::pair<int, int>>& currentPairs,
    int& bestCost,
    std::vector<std::pair<int, int>>& bestPairs) {
    // Find first unused vertex
    int first = -1;
    for (int i = 0; i < (int)used.size(); ++i) {
        if (!used[i]) { first = i; break; }
    }

    // Base case: all used => update best
    if (first == -1) {
        if (currentCost < bestCost) {
            bestCost = currentCost;
            bestPairs = currentPairs;
        }
        return;
    }

    used[first] = true;

    // Pair 'first' with any other unused j
    for (int j = first + 1; j < (int)used.size(); ++j) {
        if (used[j]) continue;

        int w = dist[first][j];
        if (w < 0) continue;

        // Prune if already worse than best
        if (currentCost + w >= bestCost) continue;

        used[j] = true;
        currentPairs.push_back({ first, j });

        recMatch(dist, used, currentCost + w, currentPairs, bestCost, bestPairs);

        currentPairs.pop_back();
        used[j] = false;
    }

    used[first] = false;
}

MatchingResult Matching::minPerfectMatching(const std::vector<int>& oddVerts,
    const std::vector<std::vector<int>>& dist) {
    const int n = (int)oddVerts.size();
    if (n == 0) return MatchingResult{};
    if (n % 2 != 0) throw std::runtime_error("Matching: oddVerts count must be even");
    if ((int)dist.size() != n) throw std::runtime_error("Matching: dist size mismatch");

    for (int i = 0; i < n; ++i) {
        if ((int)dist[i].size() != n) throw std::runtime_error("Matching: dist row size mismatch");
    }

    std::vector<bool> used(n, false);
    std::vector<std::pair<int, int>> currentPairs;

    int bestCost = std::numeric_limits<int>::max();
    std::vector<std::pair<int, int>> bestPairs;

    recMatch(dist, used, 0, currentPairs, bestCost, bestPairs);

    MatchingResult res;
    res.totalCost = bestCost;
    res.pairs = bestPairs;
    return res;
}
