#include <queue>
#include <cstring>
#include <utility>

#include "Grid.h"
#include "Cell.h"

extern int startX;
extern int startY;
extern int endX;
extern int endY;

// BFS
std::queue<std::pair<int,int>> bfsQueue;
bool visited[80][45] = {false};
bool bfsStarted = false;

std::pair<int,int> parent[80][45];
bool foundEnd = false;

// start BFS
void startBFS(int sx, int sy)
{
    // Force the grid to clean out old paths before running again
    resetSearchState();

    // Clear out the search queue completely
    while (!bfsQueue.empty()) bfsQueue.pop();

    // Clear local tracker tracking matrices
    memset(visited, false, sizeof(visited));

    foundEnd = false;

    bfsQueue.push({sx, sy});
    visited[sx][sy] = true;
    grid[sy][sx].visited = true;

    bfsStarted = true;
}

// one BFS step
void bfsStep()
{
    if (bfsQueue.empty() || foundEnd) return;

    auto [x, y] = bfsQueue.front();
    bfsQueue.pop();

    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    for (int i = 0; i < 4; i++)
    {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (nx < 0 || nx >= GRID_WIDTH || ny < 0 || ny >= GRID_HEIGHT)
            continue;

        if (visited[nx][ny] || grid[ny][nx].isWall)
            continue;

        visited[nx][ny] = true;
        grid[ny][nx].visited = true;

        parent[nx][ny] = {x, y};
        bfsQueue.push({nx, ny});

        if (grid[ny][nx].isEnd)
        {
            foundEnd = true;
            return;
        }
    }
}

// build shortest path
void buildPath(int ex, int ey)
{
    int x = ex;
    int y = ey;

    while (!(x == startX && y == startY))
    {
        grid[y][x].isPath = true;

        auto p = parent[x][y];
        x = p.first;
        y = p.second;
    }
}