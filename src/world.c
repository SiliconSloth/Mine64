#include "world.h"
#include "blocks.h"
#include "noise.h"
#include "math.h"

void generateLeafHeights(int *heights) {
  int i;
  for (i = 0; i < 25; i++) {
    heights[i] = 0;
  }

  heights[2 * 5 + 2] = 2;
  heights[1 * 5 + 2] = 2;
  heights[3 * 5 + 2] = 2;
  heights[2 * 5 + 1] = 2;
  heights[2 * 5 + 3] = 2;

  heights[1 * 5 + 1] = random(3);
  heights[1 * 5 + 3] = random(3);
  heights[3 * 5 + 1] = random(3);
  heights[3 * 5 + 3] = random(3);
  
  heights[0 * 5 + 0] = -random(3);
  heights[0 * 5 + 4] = -random(3);
  heights[4 * 5 + 0] = -random(3);
  heights[4 * 5 + 4] = -random(3);
}

void trySpawnTree(int tx, int tz) {
  u8 block;
  int ty, height;
  int x, y, z;
  int leaf_heightmap[25];

  for (ty = MAX_Y - 1; ty >= 0; ty--) {
    block = blocks[tx * MAX_Z * MAX_Y + ty * MAX_Z + tz];
    if (block == GRASS) {
      break;
    } else if (block != AIR) {
      return;
    }
  }

  blocks[tx * MAX_Z * MAX_Y + ty * MAX_Z + tz] = DIRT;

  height = random(3) + 3;
  for (y = ty + 1; y < min(ty + height + 1, MAX_Y); y++) {
    blocks[tx * MAX_Z * MAX_Y + y * MAX_Z + tz] = WOOD;
  }

  generateLeafHeights(leaf_heightmap);

  for (x = max(tx - 2, 0); x < min(tx + 3, MAX_X); x++) {
    for (z = max(tz - 2, 0); z < min(tz + 3, MAX_Z); z++) {
      for (y = ty + height - 1; y < min(ty + height + leaf_heightmap[(x - tx + 2) * 5 + (z - tz + 2)] + 1, MAX_Y); y++) {
        if (!blocks[x * MAX_Z * MAX_Y + y * MAX_Z + z]) {
          blocks[x * MAX_Z * MAX_Y + y * MAX_Z + z] = LEAVES;
        }
      }
    }
  }
}

void initWorld() {
  int x, y, z, i;
  float base, peaks;
  u8 height, block, do_sand;

  seed = (u32) osGetTime();

  for (x = 0; x < MAX_X; x++) {
    for (z = 0; z < MAX_Z; z++) {
      base = perlin2d(x, z, 0.02, 2);
      peaks = perlin2d(x, z, 0.1, 2);
      height = base * 8 + perlin2d(x, z, 0.1, 2) * 4 + peaks * peaks * peaks * peaks * 10 + 3;
      do_sand = height + base * 10 < 14;

      for (y = 0; y < MAX_Y; y++) {
        if (y < height - 3) {
          block = STONE;
        } else if (y >= height)  {
          block = AIR;
        } else if (do_sand) {
          block = SAND;
        } else if (y == height - 1) {
          block = GRASS;
        } else {
          block = DIRT;
        }

        blocks[x * MAX_Z * MAX_Y + y * MAX_Z + z] = block;
      }
    }
  }
  
  for (x = 0; x < MAX_X; x++) {
    for (z = 0; z < MAX_Z; z++) {
      if (random(1000) < perlin2d(x, z, 0.02, 2) * 8 - 2) {
        trySpawnTree(x, z);
      }
    }
  }
}