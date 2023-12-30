#ifndef WORLD_H
#define WORLD_H

#include <nusys.h>

#define MAX_X 64
#define MAX_Y 32
#define MAX_Z 64

#define CHUNK_SIZE 8

#define CHUNKS_X (MAX_X / CHUNK_SIZE)
#define CHUNKS_Y (MAX_Y / CHUNK_SIZE)
#define CHUNKS_Z (MAX_Z / CHUNK_SIZE)

#define NUM_BLOCKS (MAX_X * MAX_Y * MAX_Z)
#define NUM_CHUNKS (CHUNKS_X * CHUNKS_Y * CHUNKS_Z)

u8 blocks[NUM_BLOCKS];

void initWorld();

#endif /* WORLD_H */