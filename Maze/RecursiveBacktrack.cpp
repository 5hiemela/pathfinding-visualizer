#include "RecursiveBacktrack.h"
#include "../Grid/Grid.h"
#include <stack>
#include <algorithm> // For std::shuffle
#include <random>    // For std::mt19937

bool mazeGenerationStarted = false;

// Internal algorithm stack to track where to backtrack to
static std::stack<MazeCoordinate> mazeStack;

// Random number setup
static std::random_device rd;
static std::mt19937 rng(rd());

void initRecursiveBacktrack(int startX, int startY) {
    // Wipe out any old stack data
    while (!mazeStack.empty()) {
        mazeStack.pop();
    }

    // Clear any lingering search paths from old pathfinding runs
    resetSearchState();

    // Turn EVERY single cell on the board into a solid wall
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            grid[y][x].isWall = true;
        }
    }

    // Force our starting point to be empty, mark it visited, and push to stack
    grid[startY][startX].isWall = false;
    grid[startY][startX].visited = true;

    mazeStack.push({startX, startY});
}

bool recursiveBacktrackStep() {
    // If the stack is empty, the maze is fully generated
    if (mazeStack.empty()) {

        resetSearchState();
        return true;
    }

    // Look at the current cell at the top of the stack
    MazeCoordinate current = mazeStack.top();

    // Define the 4 directions, moving TWO steps at a time
    // {dx, dy}
    struct Direction { int dx, dy; };
    std::vector<Direction> dirs = {
        {0, -2}, // Up
        {0, 2},  // Down
        {-2, 0}, // Left
        {2, 0}   // Right
    };

    // Shuffle directions randomly to ensure the maze branches unpredictably
    std::shuffle(dirs.begin(), dirs.end(), rng);

    // Look for an unvisited neighbor 2 steps away
    for (const auto& dir : dirs) {
        int nextX = current.x + dir.dx;
        int nextY = current.y + dir.dy;

        // Check if the 2-step neighbor is completely inside the grid boundaries
        if (nextX >= 0 && nextX < GRID_WIDTH && nextY >= 0 && nextY < GRID_HEIGHT) {
            // Check if that neighbor cell hasn't been visited yet
            if (!grid[nextY][nextX].visited) {

                // Calculate where the wall is (exactly 1 step away between current and next)
                int wallX = current.x + (dir.dx / 2);
                int wallY = current.y + (dir.dy / 2);

                // Clear the wall cell and the neighbor cell
                grid[wallY][wallX].isWall = false;
                grid[nextY][nextX].isWall = false;

                // Mark the neighbor as visited so it doesn't carve into it again
                grid[nextY][nextX].visited = true;

                // Push the neighbor to our stack to move forward
                mazeStack.push({nextX, nextY});

                return false; // Step completed, maze not done yet
            }
        }
    }

    // If all 4 directions are checked and found no unvisited neighbors, it hit a dead end
    // Pop the stack to backtrack to the previous cell.
    mazeStack.pop();
    return false;
}