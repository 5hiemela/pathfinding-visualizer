#pragma once

extern bool bidirectionalStarted;

void startBidirectional(int sx, int sy, int ex, int ey);
void bidirectionalStep();
void buildBidirectionalPath(int collisionX, int collisionY);