#pragma once

// Represents a coordinate on the grid for the algorithm's internal stack
struct MazeCoordinate {
    int x, y;
};

// Sets up the grid by making everything a wall and pushing the initial cell
void initRecursiveBacktrack(int startX, int startY);

// Steps through the generation loop one cell at a time. Returns true when the maze is complete.
bool recursiveBacktrackStep();

extern bool mazeGenerationStarted;