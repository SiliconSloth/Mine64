#include <nusys.h>
#include "blocks.h"
#include "geometry.h"

#define KEEP 0
#define NOT_VISIBLE 1
#define OBSTRUCTED 2

typedef struct {
  u8 lower;
  u8 upper;
  u8 start;
  u8 block;
} Front;

typedef struct {
  Front fronts[CHUNK_SIZE];
  u8 n_fronts;
  u8 lower;
  u8 block;
  u8 fi;
  u8 status;
} Scanline;

void setFront(Front *front, u8 lower, u8 upper, u8 start, u8 block) {
  front->lower = lower;
  front->upper = upper;
  front->start = start;
  front->block = block;
}

void insertFront(Scanline *scan, u8 i, u8 lower, u8 upper, u8 start, u8 block) {
    u8 j;
    for (j = scan->n_fronts; j > i; j--) {
      scan->fronts[j] = scan->fronts[j - 1];
    }

    setFront(&scan->fronts[i], lower, upper, start, block);
    scan->n_fronts++;
}

void removeFront(Scanline *scan, u8 i) {
  u8 j;
  for (j = i; j < scan->n_fronts - 1; j++) {
    scan->fronts[j] = scan->fronts[j + 1];
  }

  scan->n_fronts--;
}

void appendQuad(QuadList *list, u8 bs, u8 bt, u8 width, u8 height, u8 block) {
  list->quads[list->n].bs = bs;
  list->quads[list->n].bt = bt;
  list->quads[list->n].width = width - 1;
  list->quads[list->n].height = height - 1;
  list->quads[list->n].block = block;
  list->n++;
}

u8 blockAt(u8 r, u8 s, u8 t, u8 axes) {
  if (axes == ZXY) {
    return blocks[s * MAX_Y * MAX_Z + t * MAX_Z + r];
  } else if (axes == XZY) {
    return blocks[r * MAX_Y * MAX_Z + t * MAX_Z + s];
  } else if (axes == YXZ) {
    if (r >= MAX_Y) {
      return 0;
    }
    return blocks[s * MAX_Y * MAX_Z + r * MAX_Z + t];
  }
}

void dropFront(Scanline *scan, QuadList *quads, u8 bs) {
  appendQuad(quads,
    scan->fronts[scan->fi].start,
    scan->fronts[scan->fi].lower,
    bs - scan->fronts[scan->fi].start,
    scan->fronts[scan->fi].upper - scan->fronts[scan->fi].lower,
    scan->fronts[scan->fi].block
  );
  removeFront(scan, scan->fi);
}

void droppingStep(Scanline *scan, QuadList *quads, u8 bs, u8 bt, u8 block, u8 cover_block) {
  if (scan->fi < scan->n_fronts && bt >= scan->fronts[scan->fi].upper) {
    if (scan->status != KEEP) {
      dropFront(scan, quads, bs);
    } else {
      scan->fi++;
    }
    scan->status = NOT_VISIBLE;
  }

  if (scan->fi < scan->n_fronts && bt >= scan->fronts[scan->fi].lower && scan->status != OBSTRUCTED && !cover_block) {
    if (block == scan->fronts[scan->fi].block) {
      scan->status = KEEP;
    } else {
      scan->status = OBSTRUCTED;
    }
  }
}

void droppingFinish(Scanline *scan, QuadList *quads, u8 bs) {
  if (scan->fi < scan->n_fronts && scan->status != KEEP) {
    dropFront(scan, quads, bs);
  }
}

void droppingPhase(Scanline *scan1, Scanline *scan2, QuadList *quads1, QuadList *quads2, u8 ct, u8 bs, u8 r, u8 s, u8 axes) {
  u8 bt, t;
  u8 block, block_f;

  scan1->fi = 0;
  scan1->status = NOT_VISIBLE;
  scan2->fi = 0;
  scan2->status = NOT_VISIBLE;

  for (bt = 0; bt < CHUNK_SIZE; bt++) {
    t = ct * CHUNK_SIZE + bt;

    block = blockAt(r, s, t, axes);
    block_f = blockAt(r + 1, s, t, axes);

    droppingStep(scan1, quads1, bs, bt, block, block_f);
    droppingStep(scan2, quads2, bs, bt, block_f, block);
  }

  droppingFinish(scan1, quads1, bs);
  droppingFinish(scan2, quads2, bs);
}

void endFront(Scanline *scan, u8 upper, u8 start) {
  if (scan-> lower != NONE) {
    insertFront(scan, scan->fi, scan->lower, upper, start, scan->block);
    scan->fi++;
    scan->lower = NONE;
  }
}

void creationStep(Scanline *scan, u8 bs, u8 bt, u8 block, u8 cover_block, u8 split_grass) {
  if (scan->fi < scan->n_fronts && bt >= scan->fronts[scan->fi].upper) {
    scan->fi++;
  }

  if (split_grass && scan->block == GRASS) {
    endFront(scan, bt, bs);
  }

  if (scan->fi < scan->n_fronts && bt >= scan->fronts[scan->fi].lower) {
    endFront(scan, bt, bs);
  } else {
    if (block && !cover_block) {
      if (block != scan->block) {
        endFront(scan, bt, bs);
      }

      if (scan->lower == NONE) {
        scan->lower = bt;
        scan->block = block;
      }
    } else {
      endFront(scan, bt, bs);
    }
  }
}

void creationFinish(Scanline *scan, u8 bs) {
  if (scan->lower != NONE) {
    insertFront(scan, scan->fi, scan->lower, CHUNK_SIZE, bs, scan->block);
    scan->lower = NONE;
  }
}

void creationPhase(Scanline *scan1, Scanline *scan2, u8 ct, u8 bs, u8 r, u8 s, u8 axes, u8 split_grass) {
  u8 bt, t;
  u8 block, block_f;

  scan1->fi = 0;
  scan1->lower = NONE;
  scan2->fi = 0;
  scan2->lower = NONE;

  for (bt = 0; bt < CHUNK_SIZE; bt++) {
    t = ct * CHUNK_SIZE + bt;

    block = blockAt(r, s, t, axes);
    block_f = blockAt(r + 1, s, t, axes);

    creationStep(scan1, bs, bt, block, block_f, split_grass);
    creationStep(scan2, bs, bt, block_f, block, split_grass);
  }

  creationFinish(scan1, bs);
  creationFinish(scan2, bs);
}

void appendAll(Scanline *scan, QuadList *quads) {
  for (scan->fi = 0; scan->fi < scan->n_fronts; scan->fi++) {
    appendQuad(quads,
      scan->fronts[scan->fi].start,
      scan->fronts[scan->fi].lower,
      CHUNK_SIZE - scan->fronts[scan->fi].start,
      scan->fronts[scan->fi].upper - scan->fronts[scan->fi].lower,
      scan->fronts[scan->fi].block
    );
  }
}

void makeChunkPlaneQuads(DualQuadList *axis_quads, u8 cr, u8 cs, u8 ct, u8 br, u8 max_r, u8 axes, u8 split_grass) {
  u8 bs, r, s, i;

  Scanline scan1, scan2;
  QuadList quads1, quads2;

  DualQuadList *both_quads;

  quads1.n = 0;
  quads2.n = 0;

  both_quads = &(axis_quads)[br];

  r = cr * CHUNK_SIZE + br;
  if (r >= max_r) {
    both_quads->n_front = 0;
    both_quads->n_back = 0;
  } else {
    scan1.n_fronts = 0;
    scan2.n_fronts = 0;
    
    for (bs = 0; bs < CHUNK_SIZE; bs++) {
      s = cs * CHUNK_SIZE + bs;
      droppingPhase(&scan1, &scan2, &quads1, &quads2, ct, bs, r, s, axes);
      creationPhase(&scan1, &scan2, ct, bs, r, s, axes, split_grass);
    }

    appendAll(&scan1, &quads1);
    appendAll(&scan2, &quads2);

    both_quads->n_front = quads1.n;
    both_quads->n_back = quads2.n;

    for (i = 0; i < quads1.n; i++) {
      both_quads->quads[i] = quads1.quads[i];
    }

    for (i = 0; i < quads2.n; i++) {
      both_quads->quads[both_quads->n_front + i] = quads2.quads[i];
    }
  }
}

void makeChunkAxisQuads(DualQuadList *axis_quads, u8 cr, u8 cs, u8 ct, u8 max_r, u8 axes, u8 split_grass) {
  u8 br;
  for (br = 0; br < CHUNK_SIZE; br++) {
    makeChunkPlaneQuads(axis_quads, cr, cs, ct, br, max_r, axes, split_grass);
  }
}

void initGeometry() {
  u8 cx, cy, cz;
  ChunkQuads *cq;
  for (cx = 0; cx < CHUNKS_X; cx++) {
    for (cy = 0; cy < CHUNKS_Y; cy++) {
      for (cz = 0; cz < CHUNKS_Z; cz++) {
        cq = &chunk_quads[cx * CHUNKS_Y * CHUNKS_Z + cy * CHUNKS_Z + cz];

        makeChunkAxisQuads(cq->x_quads, cx, cz, cy, MAX_X - 1, XZY, TRUE);
        makeChunkAxisQuads(cq->y_quads, cy, cx, cz, MAX_Y,     YXZ, FALSE);
        makeChunkAxisQuads(cq->z_quads, cz, cx, cy, MAX_Z - 1, ZXY, TRUE);
      }
    }
  }
}

void regeneratePlanes(u8 x, u8 y, u8 z, u8 regen_x, u8 regen_y, u8 regen_z) {
  u8 cx = x / CHUNK_SIZE;
  u8 cy = y / CHUNK_SIZE;
  u8 cz = z / CHUNK_SIZE;

  u8 bx = x % CHUNK_SIZE;
  u8 by = y % CHUNK_SIZE;
  u8 bz = z % CHUNK_SIZE;

  ChunkQuads *cq = &chunk_quads[cx * CHUNKS_Y * CHUNKS_Z + cy * CHUNKS_Z + cz];

  if (regen_x) {
    makeChunkPlaneQuads(cq->x_quads, cx, cz, cy, bx, MAX_X - 1, XZY, TRUE);
  }

  if (regen_y) {
    makeChunkPlaneQuads(cq->y_quads, cy, cx, cz, by, MAX_Y,     YXZ, FALSE);
  }

  if (regen_z) {
    makeChunkPlaneQuads(cq->z_quads, cz, cx, cy, bz, MAX_Z - 1, ZXY, TRUE);
  }
}

void regenerateBlock(u8 x, u8 y, u8 z) {
  regeneratePlanes(x, y, z, x < MAX_X-1, y < MAX_Y, z < MAX_Z-1);
  if (x > 0) {
    regeneratePlanes(x - 1, y, z, TRUE, FALSE, FALSE);
  }
  if (y > 0) {
    regeneratePlanes(x, y - 1, z, FALSE, TRUE, FALSE);
  }
  if (z > 0) {
    regeneratePlanes(x, y, z - 1, FALSE, FALSE, TRUE);
  }
}
