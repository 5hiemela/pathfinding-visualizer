#include <queue>
#include <cstring>
#include <utility>

#include "Bidirectional.h"
#include "../Grid/Grid.h"
#include "../Grid/Cell.h"

extern int startX;
extern int startY;
extern int endX;
extern int endY;
extern bool foundEnd;

std::queue<std::pair<int, int>> forwardQueue;
std::queue<std::pair<int, int>> backwardQueue;

bool visitedForward[80][45] = {false};
bool visitedBackward[80][45] = {false};

std::pair<int, int> parentForward[80][45];
std::pair<int, int> parentBackward[80][45];

bool bidirectionalStarted = false;
std::pair<int, int> collisionNode = {-1, -1};

// start Bidirectional
void startBidirectional(int sx, int sy, int ex, int ey)
{
    // Clean out old visual states
    resetSearchState();

    // Clear out both tracking queues completely
    while (!forwardQueue.empty()) forwardQueue.pop();
    while (!backwardQueue.empty()) backwardQueue.pop();

    // Clear local visited tracking matrices
    std::memset(visitedForward, false, sizeof(visitedForward));
    std::memset(visitedBackward, false, sizeof(visitedBackward));

    foundEnd = false;
    collisionNode = {-1, -1};

    // Initialize Forward Search (From Start)
    forwardQueue.push({sx, sy});
    visitedForward[sx][sy] = true;
    grid[sy][sx].visited = true;

    // Initialize Backward Search (From End)
    backwardQueue.push({ex, ey});
    visitedBackward[ex][ey] = true;
    grid[ey][ex].visited = true;

    bidirectionalStarted = true;
}

// one Bidirectional step
void bidirectionalStep()
{
    // Stop immediately if queues run dry or target path intersection is achieved
    if ((forwardQueue.empty() && backwardQueue.empty()) || foundEnd) {
        bidirectionalStarted = false;
        return;
    }

    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    // Forward step
    if (!forwardQueue.empty() && !foundEnd)
    {
        auto [x, y] = forwardQueue.front();
        forwardQueue.pop();

        for (int i = 0; i < 4; i++)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (nx < 0 || nx >= GRID_WIDTH || ny < 0 || ny >= GRID_HEIGHT) continue;
            if (visitedForward[nx][ny] || grid[ny][nx].isWall) continue;

            // Check if the backward search has already been here
            if (visitedBackward[nx][ny]) {
                parentForward[nx][ny] = {x, y};
                collisionNode = {nx, ny};
                foundEnd = true;
                bidirectionalStarted = false;
                return;
            }

            visitedForward[nx][ny] = true;
            grid[ny][nx].visited = true;
            parentForward[nx][ny] = {x, y};
            forwardQueue.push({nx, ny});
        }
    }

    // Backward step
    if (!backwardQueue.empty() && !foundEnd)
    {
        auto [x, y] = backwardQueue.front();
        backwardQueue.pop();

        for (int i = 0; i < 4; i++)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (nx < 0 || nx >= GRID_WIDTH || ny < 0 || ny >= GRID_HEIGHT) continue;
            if (visitedBackward[nx][ny] || grid[ny][nx].isWall) continue;

            // Check if the forward search has already been here
            if (visitedForward[nx][ny]) {
                parentBackward[nx][ny] = {x, y};
                collisionNode = {nx, ny};
                foundEnd = true;
                bidirectionalStarted = false;
                return;
            }

            visitedBackward[nx][ny] = true;
            grid[ny][nx].visited = true;
            parentBackward[nx][ny] = {x, y};
            backwardQueue.push({nx, ny});
        }
    }
}

// build path from intersection node back out to both endpoints
void buildBidirectionalPath(int collisionX, int collisionY)
{
    // Trace path from intersection point BACK to Start Node
    int x = collisionX;
    int y = collisionY;
    while (!(x == startX && y == startY)) {
        grid[y][x].isPath = true;
        auto p = parentForward[x][y];
        x = p.first;
        y = p.second;
    }

    // Trace path from intersection point FORWARD to End Node
    x = collisionX;
    y = collisionY;
    while (!(x == endX && y == endY)) {
        grid[y][x].isPath = true;
        auto p = parentBackward[x][y];
        x = p.first;
        y = p.second;
    }

    // Reinforce start node visualization flag
    grid[startY][startX].isPath = true;
}