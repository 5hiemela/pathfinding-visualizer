#pragma once

void startBFS(int sx, int sy);
void bfsStep();
void buildBFSPath(int ex, int ey);

extern bool bfsStarted;
extern bool foundEnd;

extern int startX, startY;
extern int endX, endY;