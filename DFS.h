#ifndef PATHFINDINGVISUALIZER_DFS_H
#define PATHFINDINGVISUALIZER_DFS_H

extern bool dfsStarted;

void startDFS(int sx, int sy);
void dfsStep();
void buildDFSPath(int ex, int ey);

#endif //PATHFINDINGVISUALIZER_DFS_H