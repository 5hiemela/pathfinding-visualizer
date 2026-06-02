#include "Grid.h"

const int GRID_WIDTH = 80;
const int GRID_HEIGHT = 45;

std::vector<std::vector<Cell>> grid;

void initializeGrid()
{
    grid.resize(GRID_HEIGHT);
    for (int y = 0; y < GRID_HEIGHT; y++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            grid[y].push_back(Cell{x, y});
        }
    }
}

void resetSearchState()
{
    for (int y = 0; y < GRID_HEIGHT; y++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            grid[y][x].visited = false;
            grid[y][x].isPath = false;
        }
    }
}

void clearAllWalls()
{
    for (int y = 0; y < GRID_HEIGHT; y++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            grid[y][x].isWall = false;

            // Reset terrain back to default unweighted state
            grid[y][x].isSand = false;
            grid[y][x].isMud = false;
            grid[y][x].weight = 1;
        }
    }
}