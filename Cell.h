#pragma once

struct Cell {
    int x, y;

    bool isWall = false;
    bool isStart = false;
    bool isEnd = false;

    bool visited = false;
    bool isPath = false;
};