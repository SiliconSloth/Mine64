#include <nusys.h>
#include "camera.h"
#include "player.h"
#include "math.h"
#include "graphics.h"
#include "geometry.h"

#define FOV_Y 60
#define FOV_RATIO ((float) SCREEN_WD / (float) SCREEN_HT)
#define FOV_X (FOV_Y * FOV_RATIO)

#define NUM_CULL_LINES 4

#define ALL_ACCEPT ((1 << NUM_CULL_LINES) - 1)

typedef struct {
  float a;
  float b;
  float c;
} Line2D;

static Mtx projection_matrix;
static u16 perspective_norm;

static Mtx cam_rotate2;
static Mtx cam_rotate;
static Mtx cam_translate;

static Line2D cull_lines[NUM_CULL_LINES];

static u8 point_sides[CHUNKS_X + 1][CHUNKS_Z + 1];

void initCamera() {
  guPerspective(&projection_matrix, &perspective_norm,
	  FOV_Y, FOV_RATIO,
	  10, 8000, 1.0);
}

void makeCullLine(Line2D *line, Vector3 normal) {
  Vector3 cam_b = {cam.x / BLOCK_SIZE, cam.y / BLOCK_SIZE, cam.z / BLOCK_SIZE};

  line->a = normal.x;
  line->b = normal.z;
  line->c = -dot(normal, cam_b);
  if (pitch < 180) {
    line->c += MAX_Y * normal.y;
  }
}

void makeHorizontalCullLine(Line2D *line, float side) {
  Vector3 normal = {side, 0, 0};

  normal = rotateY(normal, side * FOV_X / 2);
  normal = rotateX(normal, pitch);
  normal = rotateY(normal, -yaw);

  makeCullLine(line, normal);
}

void makeVerticalCullLine(Line2D *line, float side) {
  Vector3 normal = {0, side, 0};

  if (pitch < 90 - FOV_Y / 2 || pitch > 270 + FOV_Y / 2) {
    normal = rotateX(normal, side * 90);
  } else {
    normal = rotateX(normal, pitch + side * FOV_Y / 2);
  }
  normal = rotateY(normal, -yaw);

  makeCullLine(line, normal);
}

void updateVisibleColumns() {
  u8 cx, cz, i;
  u8 s1, s2, s3, s4;

  makeHorizontalCullLine(&cull_lines[0], -1);
  makeHorizontalCullLine(&cull_lines[1], 1);
  makeVerticalCullLine(&cull_lines[2], -1);
  makeVerticalCullLine(&cull_lines[3], 1);

  for (cx = 0; cx <= CHUNKS_X; cx++) {
    for(cz = 0; cz <= CHUNKS_Z; cz++) {
      point_sides[cx][cz] = 0;
      for (i = 0; i < NUM_CULL_LINES; i++) {
        if (cx * CHUNK_SIZE * cull_lines[i].a + cz * CHUNK_SIZE * cull_lines[i].b + cull_lines[i].c <= 0) {
          point_sides[cx][cz] += 1 << i;
        }
      }
    }
  }

  for (cx = 0; cx < CHUNKS_X; cx++) {
    for(cz = 0; cz < CHUNKS_Z; cz++) {
      s1 = point_sides[cx][cz];
      s2 = point_sides[cx + 1][cz];
      s3 = point_sides[cx][cz + 1];
      s4 = point_sides[cx + 1][cz + 1];

      visible_columns[cx * CHUNKS_Z + cz] = (s1 | s2 | s3 | s4) == ALL_ACCEPT;
    }
  }
}

void updateCameraMatrices() {
  guTranslate(&cam_translate, -cam.x, -cam.y, -cam.z);
  guRotateRPY(&cam_rotate, 0.0, -yaw, 0.0);
  guRotateRPY(&cam_rotate2, -pitch, 0.0, 0.0);
}

void loadCameraMatrices() {
  gSPMatrix(dlp++, OS_K0_TO_PHYSICAL(&projection_matrix),
		G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
  gSPPerspNormalize(dlp++, perspective_norm);

  gSPMatrix(dlp++, OS_K0_TO_PHYSICAL(&cam_rotate2),
		G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);

  gSPMatrix(dlp++, OS_K0_TO_PHYSICAL(&cam_rotate),
		G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);

  gSPMatrix(dlp++, OS_K0_TO_PHYSICAL(&cam_translate),
		G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
}