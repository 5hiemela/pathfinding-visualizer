#pragma once

#include <vector>

// Represents a wall candidate that connects a visited cell to an unvisited cell
struct PrimWall {
    int wallX, wallY;     // The actual wall cell to carve through (1 step away)
    int targetX, targetY; // The unvisited neighbor cell (2 steps away)
};

void initPrims(int startX, int startY);
bool primsStep();

extern bool mazeGenerationStarted;