#pragma once

struct Cell {
    int x, y;

    bool isWall = false;
    bool isStart = false;
    bool isEnd = false;

    // Terrain types
    bool isSand = false;
    bool isMud = false;
    int weight = 1; // Default movement cost

    bool visited = false;
    bool isPath = false;
};