#include "math.h"
#include "graphics.h"

u32 random(u32 max) {
    seed = (seed << 2) + 2;

    seed *= (seed + 1);
    seed = seed >> 2;

    return seed % max;
}

float min(float a, float b) {
  return a < b? a : b;
}

float max(float a, float b) {
  return a > b? a : b;
}

int floor(float v) {
  int i = v;
  if (i == v || v > 0) {
    return i;
  } else {
    return i - 1;
  }
}

float tanf(float angle) {
  return sinf(angle) / cosf(angle);
}

float * at(Vector3 *v, int i) {
  switch (i) {
    case 0:
      return &(v->x);
    case 1:
      return &(v->y);
    case 2:
      return &(v->z);
  }
}

int * ati(Vector3i *v, int i) {
  switch (i) {
    case 0:
      return &(v->x);
    case 1:
      return &(v->y);
    case 2:
      return &(v->z);
  }
}

Vector3 add(Vector3 a, Vector3 b) {
  Vector3 out = {a.x + b.x, a.y + b.y, a.z + b.z};
  return out;
}

Vector3i addi(Vector3i a, Vector3i b) {
  Vector3i out = {a.x + b.x, a.y + b.y, a.z + b.z};
  return out;
}

Vector3 mul(Vector3 a, float b) {
  Vector3 out = {a.x * b, a.y * b, a.z * b};
  return out;
}

Vector3 div(Vector3 a, float b) {
  Vector3 out = {a.x / b, a.y / b, a.z / b};
  return out;
}

Vector3i divToInt(Vector3 a, float b) {
  Vector3i out = {a.x / b, a.y / b, a.z / b};
  return out;
}

float dot(Vector3 a, Vector3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 rotateX(Vector3 v, float angle) {
  Vector3 out = {
    v.x,
    v.y * cosf(angle * M_DTOR) - v.z * sinf(angle * M_DTOR),
    v.y * sinf(angle * M_DTOR) + v.z * cosf(angle * M_DTOR)
  };
  return out;
}

Vector3 rotateY(Vector3 v, float angle) {
  Vector3 out = {
    v.x * cosf(angle * M_DTOR) - v.z * sinf(angle * M_DTOR),
    v.y, 
    v.x * sinf(angle * M_DTOR) + v.z * cosf(angle * M_DTOR)
  };
  return out;
}

void rayStepAxis(float origin, float direction, int block, float *t, Vector3i *step, int axis) {
  float nt;
  if (direction < 0) {
    nt = (block * BLOCK_SIZE - origin) / direction;
    if (nt < *t) {
      *t = nt;

      *ati(step, axis) = -1;
      *ati(step, (axis + 1) % 3) = 0;
      *ati(step, (axis + 2) % 3) = 0;
    }
  } else if (direction > 0) {
    nt = ((block + 1) * BLOCK_SIZE - origin) / direction;
    if (nt < *t) {
      *t = nt;

      *ati(step, axis) = 1;
      *ati(step, (axis + 1) % 3) = 0;
      *ati(step, (axis + 2) % 3) = 0;
    }
  }
}