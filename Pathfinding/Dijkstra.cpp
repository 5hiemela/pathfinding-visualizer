#include <queue>
#include <vector>
#include <utility>

#include "Dijkstra.h"
#include "../Grid/Grid.h"
#include "../Grid/Cell.h"

// Bring in global coordinates and flags from main/grid system
extern int startX;
extern int startY;
extern int endX;
extern int endY;
extern bool foundEnd;

// Custom struct for Min-Priority Queue elements
struct DijkstraNode {
    int x, y;
    int dist;

    // Overload the greater-than operator so the priority queue acts as a MIN-HEAP
    bool operator>(const DijkstraNode& other) const {
        return dist > other.dist;
    }
};

std::priority_queue<DijkstraNode, std::vector<DijkstraNode>, std::greater<DijkstraNode>> dijkstraPQ;
int distances[80][45]; 
bool dijkstraStarted = false;

std::pair<int, int> dijkstraParent[80][45];

// Infinite distance constant
const int INF = 1000000;

// start Dijkstra
void startDijkstra(int sx, int sy)
{
    // Force the grid to clean out old paths before running again
    resetSearchState();

    // Clear out the priority queue completely
    while (!dijkstraPQ.empty()) dijkstraPQ.pop();

    // Initialize all distances to infinity
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            distances[x][y] = INF;
        }
    }

    foundEnd = false;

    // Setup start node configuration
    distances[sx][sy] = 0;
    grid[sy][sx].visited = true; // Visited state triggers rendering loop
    dijkstraPQ.push(DijkstraNode{sx, sy, 0});

    dijkstraStarted = true;
}

// one Dijkstra step
void dijkstraStep()
{
    // If the priority queue runs dry hits the target, stops immediately
    if (dijkstraPQ.empty() || foundEnd) {
        dijkstraStarted = false;
        return;
    }

    // Grab the cell with the lowest accumulated cost from the top of the Min-Heap
    auto current = dijkstraPQ.top();
    dijkstraPQ.pop();

    int x = current.x;
    int y = current.y;

    // Stale element check
    if (current.dist > distances[x][y]) return;

    grid[y][x].visited = true;

    // Check if we popped the end node
    if (grid[y][x].isEnd) {
        foundEnd = true;
        dijkstraStarted = false;
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

        // CRITICAL STEP: Cost calculation incorporating cell terrain properties
        // Normal cell = +1, Sand = +3, Mud = +5
        int newDist = distances[x][y] + grid[ny][nx].weight;

        // If this path is cheaper than what it found before, update it
        if (newDist < distances[nx][ny]) {
            distances[nx][ny] = newDist;
            dijkstraParent[nx][ny] = {x, y};
            dijkstraPQ.push(DijkstraNode{nx, ny, newDist});
        }
    }
}

// build shortest weighted path
void buildDijkstraPath(int ex, int ey)
{
    int x = ex;
    int y = ey;

    while (!(x == startX && y == startY))
    {
        grid[y][x].isPath = true;

        auto p = dijkstraParent[x][y];
        x = p.first;
        y = p.second;
    }
}