#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <nusys.h>
#include "world.h"

#define ZXY 0
#define XZY 1
#define YXZ 2

#define FRONT  0
#define LEFT   1
#define BACK   2
#define RIGHT  3
#define TOP    4
#define BOTTOM 5

#define NONE 255
#define ALL  254

typedef struct {
  u16 bs:3;
  u16 bt:3;
  u16 width:3;
  u16 height:3;
  u16 block:4;
} Quad;

typedef struct {
  u8 n;
  Quad quads[CHUNK_SIZE * CHUNK_SIZE];
} QuadList;

typedef struct {
  u8 n_front;
  u8 n_back;
  Quad quads[CHUNK_SIZE * CHUNK_SIZE];
} DualQuadList;

typedef struct {
  DualQuadList x_quads[CHUNK_SIZE];
  DualQuadList y_quads[CHUNK_SIZE];
  DualQuadList z_quads[CHUNK_SIZE];
} ChunkQuads;

ChunkQuads chunk_quads[NUM_CHUNKS];

void initGeometry();
void regenerateBlock(u8 x, u8 y, u8 z);

#endif /* GEOMETRY_H */