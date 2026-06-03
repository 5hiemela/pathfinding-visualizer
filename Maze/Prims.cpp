#include "Prims.h"
#include "../Grid/Grid.h"
#include <algorithm>
#include <random>

// The central pool of all valid walls it can currently choose to carve next
static std::vector<PrimWall> frontier;

static std::random_device rd;
static std::mt19937 rng(rd());

void initPrims(int startX, int startY) {
    frontier.clear();
    resetSearchState();

    // Fill the grid with solid walls
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            grid[y][x].isWall = true;
        }
    }

    // Clear the starting cell and mark it visited
    grid[startY][startX].isWall = false;
    grid[startY][startX].visited = true;

    // Look in 4 directions around the start cell to find the first batch of frontier walls
    int dx[] = {0, 0, -2, 2};
    int dy[] = {-2, 2, 0, 0};

    for (int i = 0; i < 4; i++) {
        int nextX = startX + dx[i];
        int nextY = startY + dy[i];

        if (nextX >= 0 && nextX < GRID_WIDTH && nextY >= 0 && nextY < GRID_HEIGHT) {
            frontier.push_back({
                startX + (dx[i] / 2), startY + (dy[i] / 2), // The wall between them
                nextX, nextY                                // The target cells 2 steps away
            });
        }
    }
}

bool primsStep() {
    // If there are no more frontier walls to evaluate, the maze is done growing
    if (frontier.empty()) {
        resetSearchState();
        return true;
    }

    // Grab a completely random index from the frontier pool
    std::uniform_int_distribution<size_t> dist(0, frontier.size() - 1);
    size_t randIdx = dist(rng);
    PrimWall chosen = frontier[randIdx];

    // Erase it from the vector
    std::swap(frontier[randIdx], frontier.back());
    frontier.pop_back();

    // Check if the target cell on the other side of that wall hasn't been visited yet
    if (!grid[chosen.targetY][chosen.targetX].visited) {

        // Carve through both the wall and the cell
        grid[chosen.wallY][chosen.wallX].isWall = false;
        grid[chosen.targetY][chosen.targetX].isWall = false;
        grid[chosen.targetY][chosen.targetX].visited = true;

        // From this newly incorporated cell, scan for its neighbors to find new frontier paths
        int dx[] = {0, 0, -2, 2};
        int dy[] = {-2, 2, 0, 0};

        for (int i = 0; i < 4; i++) {
            int nextX = chosen.targetX + dx[i];
            int nextY = chosen.targetY + dy[i];

            if (nextX >= 0 && nextX < GRID_WIDTH && nextY >= 0 && nextY < GRID_HEIGHT) {
                // If that cell hasn't been visited yet, it's a new frontier candidate
                if (!grid[nextY][nextX].visited) {
                    frontier.push_back({
                        chosen.targetX + (dx[i] / 2), chosen.targetY + (dy[i] / 2),
                        nextX, nextY
                    });
                }
            }
        }
    }

    return false;
}