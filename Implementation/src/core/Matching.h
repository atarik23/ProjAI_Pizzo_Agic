#pragma once
#include <vector>
#include <utility>

/*
  Finds minimum-weight perfect matching on a small set of vertices.
  We assume:
    - oddVerts contains the vertex IDs in the original graph (e.g. 0,2,4,6)
    - dist[i][j] is the shortest-path distance between oddVerts[i] and oddVerts[j]
      (so i and j are indices into oddVerts, not graph vertices).

  Output:
    - totalCost: minimal sum
    - pairs: list of matched pairs as indices into oddVerts (i,j)
*/
struct MatchingResult {
    int totalCost = 0;
    std::vector<std::pair<int, int>> pairs;
};

class Matching {
public:
    static MatchingResult minPerfectMatching(const std::vector<int>& oddVerts,
        const std::vector<std::vector<int>>& dist);
};
