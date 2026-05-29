#include <stack>
#include <cstring>
#include <utility>

#include "DFS.h"
#include "Grid.h"
#include "Cell.h"

// Bring in the global coordinates defined in your main/grid system
extern int startX;
extern int startY;
extern int endX;
extern int endY;

// DFS Globals matching your architecture style
std::stack<std::pair<int, int>> dfsStack;
bool dfsVisited[80][45] = {false}; // Separate visited array specifically for DFS
bool dfsStarted = false;

std::pair<int, int> dfsParent[80][45]; // Separate parent map specifically for DFS
extern bool foundEnd; // Uses your global foundEnd flag from main/BFS

// start DFS
void startDFS(int sx, int sy)
{
    // Force the grid to clean out old paths before running again
    resetSearchState();

    // Clear out the search stack completely
    while (!dfsStack.empty()) dfsStack.pop();

    // Clear local tracking matrices
    memset(dfsVisited, false, sizeof(dfsVisited));

    foundEnd = false;

    // Push the initial node
    dfsStack.push({sx, sy});
    dfsVisited[sx][sy] = true;
    grid[sy][sx].visited = true;

    dfsStarted = true;
}

// one DFS step
void dfsStep()
{
    // If the stack runs dry or we hit the target, halt immediately
    if (dfsStack.empty() || foundEnd) {
        dfsStarted = false;
        return;
    }

    // Pop from the TOP of the stack (Last-In, First-Out)
    auto [x, y] = dfsStack.top();
    dfsStack.pop();

    // The 4 cardinal directions matching your BFS layout
    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    for (int i = 0; i < 4; i++)
    {
        int nx = x + dx[i];
        int ny = y + dy[i];

        // Bounds validation checking your hardcoded dimensions
        if (nx < 0 || nx >= GRID_WIDTH || ny < 0 || ny >= GRID_HEIGHT)
            continue;

        // Skip if already evaluated or blocked by a wall
        if (dfsVisited[nx][ny] || grid[ny][nx].isWall)
            continue;

        // Mark state instantly
        dfsVisited[nx][ny] = true;
        grid[ny][nx].visited = true;

        // Log history node mapping for path traversal drawing later
        dfsParent[nx][ny] = {x, y};
        dfsStack.push({nx, ny});

        // Destination match evaluation check
        if (grid[ny][nx].isEnd)
        {
            foundEnd = true;
            dfsStarted = false;
            return;
        }
    }
}

// build DFS path (reuses your same logic structure but reads from dfsParent)
void buildDFSPath(int ex, int ey)
{
    int x = ex;
    int y = ey;

    while (!(x == startX && y == startY))
    {
        grid[y][x].isPath = true;

        auto p = dfsParent[x][y];
        x = p.first;
        y = p.second;
    }
}