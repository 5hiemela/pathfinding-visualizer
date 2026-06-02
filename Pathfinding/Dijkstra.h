#pragma once

extern bool dijkstraStarted;

void startDijkstra(int sx, int sy);
void dijkstraStep();
void buildDijkstraPath(int ex, int ey);