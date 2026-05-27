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