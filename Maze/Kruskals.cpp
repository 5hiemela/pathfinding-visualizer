#include "Kruskals.h"
#include "../Grid/Grid.h"
#include <algorithm>
#include <random>

static std::vector<KruskalWall> wallPool;
static DisjointSet ds;

static std::random_device rd;
static std::mt19937 rng(rd());

// Initialize Disjoint Set vectors
void DisjointSet::init(int width, int height) {
    parentX.assign(height, std::vector<int>(width, 0));
    parentY.assign(height, std::vector<int>(width, 0));
    rank.assign(height, std::vector<int>(width, 0));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            parentX[y][x] = x;
            parentY[y][x] = y;
            rank[y][x] = 0;
        }
    }
}

// Find with Path Compression
std::pair<int, int> DisjointSet::find(int x, int y) {
    if (parentX[y][x] == x && parentY[y][x] == y) {
        return {x, y};
    }
    // Path Compression: Make parent point directly to the root representative
    std::pair<int, int> root = find(parentX[y][x], parentY[y][x]);
    parentX[y][x] = root.first;
    parentY[y][x] = root.second;
    return root;
}

// Union by Rank: Returns true if sets were successfully merged
bool DisjointSet::unionSets(int x1, int y1, int x2, int y2) {
    std::pair<int, int> root1 = find(x1, y1);
    std::pair<int, int> root2 = find(x2, y2);

    if (root1 != root2) {
        // Attach shorter tree under root of deeper tree
        if (rank[root1.second][root1.first] < rank[root2.second][root2.first]) {
            parentX[root1.second][root1.first] = root2.first;
            parentY[root1.second][root1.first] = root2.second;
        } else if (rank[root1.second][root1.first] > rank[root2.second][root2.first]) {
            parentX[root2.second][root2.first] = root1.first;
            parentY[root2.second][root2.first] = root1.second;
        } else {
            parentX[root2.second][root2.first] = root1.first;
            parentY[root2.second][root2.first] = root1.second;
            rank[root1.second][root1.first]++;
        }
        return true;
    }
    return false;
}

void initKruskals(int startX, int startY) {
    wallPool.clear();
    resetSearchState();
    ds.init(GRID_WIDTH, GRID_HEIGHT);

    // Fill entire grid with solid walls
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            grid[y][x].isWall = true;
        }
    }

    // Populate the lottery pool with all internal walls between odd cells
    for (int y = 1; y < GRID_HEIGHT - 1; y += 2) {
        for (int x = 1; x < GRID_WIDTH - 1; x += 2) {
            // Clear the actual room cells
            grid[y][x].isWall = false;

            // Check Horizontal Wall Candidate (to the right)
            if (x + 2 < GRID_WIDTH - 1) {
                wallPool.push_back({x + 1, y, x, y, x + 2, y});
            }
            // Check Vertical Wall Candidate (below)
            if (y + 2 < GRID_HEIGHT - 1) {
                wallPool.push_back({x, y + 1, x, y, x, y + 2});
            }
        }
    }

    // Shuffle the wall pool upfront so we can pop sequentially for O(1) step execution
    std::shuffle(wallPool.begin(), wallPool.end(), rng);
}

bool kruskalsStep() {
    if (wallPool.empty()) {
        resetSearchState();
        return true;
    }

    // Grab the last wall from our shuffled pool (O(1) pop optimization)
    KruskalWall chosen = wallPool.back();
    wallPool.pop_back();

    // Check if cells on either side belong to different sets
    if (ds.unionSets(chosen.cell1X, chosen.cell1Y, chosen.cell2X, chosen.cell2Y)) {
        // Smashes down the intermediate wall cell
        grid[chosen.wallY][chosen.wallX].isWall = false;
    }

    return false;
}