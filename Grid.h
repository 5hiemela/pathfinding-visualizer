#pragma once

#include <vector>
#include "Cell.h"

extern const int GRID_WIDTH;
extern const int GRID_HEIGHT;

extern std::vector<std::vector<Cell>> grid;

void initializeGrid();