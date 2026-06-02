#include <queue>
#include <vector>
#include <utility>
#include <cmath> // For std::abs

#include "AStar.h"
#include "../Grid/Grid.h"
#include "../Grid/Cell.h"

extern int startX;
extern int startY;
extern int endX;
extern int endY;
extern bool foundEnd;

// Custom struct for Min-Priority Queue elements
struct AStarNode {
    int x, y;
    int gCost; // Distance from start node
    int fCost; // gCost + hCost (Total estimated cost)

    // Overload the greater-than operator so the priority queue acts as a MIN-HEAP
    bool operator>(const AStarNode& other) const {
        return fCost > other.fCost;
    }
};

std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> astarPQ;
int gCosts[80][45];
bool astarStarted = false;

std::pair<int, int> astarParent[80][45];

// Infinite cost constant
const int INF = 1000000;

// Helper function to calculate Manhattan Distance heuristic (hCost)
int getManhattanDistance(int x1, int y1, int x2, int y2) {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}

// start A*
void startAStar(int sx, int sy)
{
    // Force the grid to clean out old paths before running again
    resetSearchState();

    // Clear out the priority queue completely
    while (!astarPQ.empty()) astarPQ.pop();

    // Initialize all gCosts to infinity
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            gCosts[x][y] = INF;
        }
    }

    foundEnd = false;

    // Setup start node configurations
    gCosts[sx][sy] = 0;
    grid[sy][sx].visited = true;

    int hCost = getManhattanDistance(sx, sy, endX, endY);
    astarPQ.push(AStarNode{sx, sy, 0, hCost});

    astarStarted = true;
}

// one A* step
void astarStep()
{
    // If the priority queue runs dry or it hits the target, stop immediately
    if (astarPQ.empty() || foundEnd) {
        astarStarted = false;
        return;
    }

    // Grab the cell with the lowest estimated total cost (fCost) from the top of the Min-Heap
    auto current = astarPQ.top();
    astarPQ.pop();

    int x = current.x;
    int y = current.y;

    // Stale element check
    if (current.gCost > gCosts[x][y]) return;

    // Mark visual state for active assessment
    grid[y][x].visited = true;

    // Check if it popped the end node
    if (grid[y][x].isEnd) {
        foundEnd = true;
        astarStarted = false;
        return;
    }

    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    for (int i = 0; i < 4; i++)
    {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (nx < 0 || nx >= GRID_WIDTH || ny < 0 || ny >= GRID_HEIGHT)
            continue;

        // Skip if blocked by a wall
        if (grid[ny][nx].isWall)
            continue;

        // Calculate tentative gCost incorporating cell terrain properties
        int tentativeGCost = gCosts[x][y] + grid[ny][nx].weight;

        // If this path to the neighbor is cheaper than previously calculated
        if (tentativeGCost < gCosts[nx][ny]) {
            gCosts[nx][ny] = tentativeGCost;
            astarParent[nx][ny] = {x, y};

            // fCost = gCost (actual distance) + hCost (estimated heuristic distance to end)
            int hCost = getManhattanDistance(nx, ny, endX, endY);
            int fCost = tentativeGCost + hCost;

            astarPQ.push(AStarNode{nx, ny, tentativeGCost, fCost});
        }
    }
}

// build shortest weighted path
void buildAStarPath(int ex, int ey)
{
    int x = ex;
    int y = ey;

    while (!(x == startX && y == startY))
    {
        grid[y][x].isPath = true;

        auto p = astarParent[x][y];
        x = p.first;
        y = p.second;
    }
}