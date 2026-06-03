#pragma once

#include <vector>

// Represents an internal wall candidate sitting between two walkable cells
struct KruskalWall {
    int wallX, wallY;   // The wall cell to potentially carve through
    int cell1X, cell1Y; // The odd-coordinate cell on one side
    int cell2X, cell2Y; // The odd-coordinate cell on the other side
};

// Handles the Disjoint-Set (Union-Find) logic
struct DisjointSet {
    // Flattened 2D grid vectors to track parent pointers and tree ranks
    std::vector<std::vector<int>> parentX;
    std::vector<std::vector<int>> parentY;
    std::vector<std::vector<int>> rank;

    void init(int width, int height);
    std::pair<int, int> find(int x, int y);
    bool unionSets(int x1, int y1, int x2, int y2);
};

void initKruskals(int startX, int startY);
bool kruskalsStep();

extern bool mazeGenerationStarted;