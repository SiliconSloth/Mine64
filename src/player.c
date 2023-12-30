#include <nusys.h>
#include "player.h"
#include "main.h"
#include "math.h"
#include "blocks.h"
#include "graphics.h"
#include "geometry.h"

#define START_X 32
#define START_Z 48

#define STICK_DAMPER 22

#define MOVE_SPEED (BLOCK_SIZE / 8)
#define JUMP_SPEED (BLOCK_SIZE / 4.5)
#define TERMINAL_SPEED (BLOCK_SIZE / 2)
#define GRAVITY (BLOCK_SIZE / 40)

#define BOX_RADIUS 0.35
#define BOX_HEIGHT 1.8
#define EYE_HEIGHT 1.5

static Vector3 bounding_box[] = {
  {-BOX_RADIUS, -EYE_HEIGHT, -BOX_RADIUS},
  {-BOX_RADIUS, -EYE_HEIGHT,  BOX_RADIUS},
  {-BOX_RADIUS, BOX_HEIGHT - EYE_HEIGHT, -BOX_RADIUS},
  {-BOX_RADIUS, BOX_HEIGHT - EYE_HEIGHT, BOX_RADIUS},
  { BOX_RADIUS, -EYE_HEIGHT, -BOX_RADIUS},
  { BOX_RADIUS, -EYE_HEIGHT,  BOX_RADIUS},
  { BOX_RADIUS, BOX_HEIGHT - EYE_HEIGHT, -BOX_RADIUS},
  { BOX_RADIUS, BOX_HEIGHT - EYE_HEIGHT, BOX_RADIUS}
};

static NUContData contData[1];

static float y_velocity = 0;

static u8 block_dec_held = FALSE;
static u8 block_inc_held = FALSE;

void initPlayer() {
  int y;

  pitch = 0;
  yaw = 0;

  held_block = COBBLESTONE;

  cam.x = (START_X + 0.5) * BLOCK_SIZE;
  cam.z = (START_Z + 0.5) * BLOCK_SIZE;

  for (y = MAX_Y - 1; y >= 0; y--) {
    if (blocks[START_X * MAX_Y * MAX_Z + y * MAX_Z + START_Z]) {
      cam.y = (y + 1 + EYE_HEIGHT) * BLOCK_SIZE;
      break;
    }
  }
}

void updateTargetBlock() {
  float t, nt;
  Vector3i step;
  Vector3 direction = {0, 0, -1};
  direction = rotateX(direction, pitch);
  direction = rotateY(direction, -yaw);

  target_x = cam.x / BLOCK_SIZE;
  target_y = cam.y / BLOCK_SIZE;
  target_z = cam.z / BLOCK_SIZE;

  target_present = TRUE;
  while (target_x >= MAX_X || target_y >= MAX_Y || target_z >= MAX_Z ||
    !blocks[target_x * MAX_Y * MAX_Z + target_y * MAX_Z + target_z]) {
    
    t = 9999;

    rayStepAxis(cam.x, direction.x, target_x, &t, &step, 0);
    rayStepAxis(cam.y, direction.y, target_y, &t, &step, 1);
    rayStepAxis(cam.z, direction.z, target_z, &t, &step, 2);

    if (t > BLOCK_SIZE * 6 || (target_x == 0 && step.x < 0) || (target_y == 0 && step.y < 0) || (target_z == 0 && step.z < 0)) {
      target_present = FALSE;
      break;
    } else {
      target_x += step.x;
      target_y += step.y;
      target_z += step.z;
    }
  }

  if (target_present) {
    build_offset_x = -step.x;
    build_offset_y = -step.y;
    build_offset_z = -step.z;
  }
}

void boxBlockRange(Vector3 pos, Vector3i *min_block, Vector3i *max_block) {
  int i, x, y, z;
  Vector3 v;
  Vector3 box_min = {9999, 9999, 9999};
  Vector3 box_max = {-9999, -9999, -9999};

  for (i = 0; i < 8; i++) {
    v = add(pos, bounding_box[i]);

    box_min.x = min(box_min.x, v.x);
    box_min.y = min(box_min.y, v.y);
    box_min.z = min(box_min.z, v.z);

    box_max.x = max(box_max.x, v.x);
    box_max.y = max(box_max.y, v.y);
    box_max.z = max(box_max.z, v.z);
  }

  min_block->x = floor(box_min.x);
  min_block->y = floor(box_min.y);
  min_block->z = floor(box_min.z);

  max_block->x = floor(box_max.x);
  max_block->y = floor(box_max.y);
  max_block->z = floor(box_max.z);
}

u8 boxObstructed(Vector3 pos, int override_axis, int override_block) {
  int x, y, z;
  Vector3i min_block, max_block;

  boxBlockRange(pos, &min_block, &max_block);

  *ati(&min_block, override_axis) = override_block;
  *ati(&max_block, override_axis) = override_block;

  for (x = min_block.x; x <= max_block.x; x++) {
    for (y = min_block.y; y <= max_block.y; y++) {
      for (z = min_block.z; z <= max_block.z; z++) {
        if (x < 0 || y < 0 || z < 0 || x >= MAX_X || z >= MAX_Z ||
            (y < MAX_Y && blocks[x * MAX_Y * MAX_Z + y * MAX_Z + z])) {
          return TRUE;
        }
      }
    }
  }

  return FALSE;
}

float detectCollision(Vector3 velocity, float max_t, int * collision_axis) {
  float t, nt;
  Vector3i step;
  int step_axis;

  Vector3 pos;
  Vector3 origin = add(cam, mul(bounding_box[(velocity.x > 0) * 4 + (velocity.y > 0) * 2 + (velocity.z > 0)], BLOCK_SIZE));
  Vector3i block = divToInt(origin, BLOCK_SIZE);

  while (TRUE) {
    t = 1;

    rayStepAxis(origin.x, velocity.x, block.x, &t, &step, 0);
    rayStepAxis(origin.y, velocity.y, block.y, &t, &step, 1);
    rayStepAxis(origin.z, velocity.z, block.z, &t, &step, 2);

    if (t >= max_t) {
      return max_t;
    }

    if (step.x != 0) {
      step_axis = 0;
    } else if (step.y != 0) {
      step_axis = 1;
    } else {
      step_axis = 2;
    }
    
    block = addi(block, step);
    pos = div(add(cam, mul(velocity, t)), BLOCK_SIZE);

    if (boxObstructed(pos, step_axis, *ati(&block, step_axis))) {
      *collision_axis = step_axis;
      return t;
    }
  }
}

void placeBlock(u8 x, u8 y, u8 z) {
  int bx, by, bz;
  Vector3i min_block, max_block;

  if (x >= MAX_X || y >= MAX_Y || z >= MAX_Z) {
    return;
  }

  boxBlockRange(div(cam, BLOCK_SIZE), &min_block, &max_block);

  for (bx = min_block.x; bx <= max_block.x; bx++) {
    for (by = min_block.y; by <= max_block.y; by++) {
      for (bz = min_block.z; bz <= max_block.z; bz++) {
        if (bx == x && by == y && bz == z) {
          return;
        }
      }
    }
  }

  blocks[x * MAX_Y * MAX_Z + y * MAX_Z + z] = held_block;
  regenerateBlock(x, y, z);
  makeDisplayListsAt(x, z);
}

void breakBlock(u8 x, u8 y, u8 z) {
  blocks[x * MAX_Y * MAX_Z + y * MAX_Z + z] = AIR;
  regenerateBlock(x, y, z);
  makeDisplayListsAt(x, z);
}

u8 onGround() {
  int x, y, z;
  Vector3i min_block, max_block;

  Vector3 low_pos = div(cam, BLOCK_SIZE);
  low_pos.y -= 0.01;

  boxBlockRange(low_pos, &min_block, &max_block);

  y = min_block.y;
  for (x = min_block.x; x <= max_block.x; x++) {
    for (z = min_block.z; z <= max_block.z; z++) {
      if (x < 0 || y < 0 || z < 0 || x >= MAX_X || z >= MAX_Z ||
          (y < MAX_Y && blocks[x * MAX_Y * MAX_Z + y * MAX_Z + z])) {
        return TRUE;
      }
    }
  }

  return FALSE;
}

void updatePlayer() {  
  Vector3 velocity = {0, 0, 0};
  float t;
  float t_total = 0;
  int collision_axis = 0;

  nuContDataGetEx(contData, 0);

  if (in_menu) {
    if (contData[0].button) {
      generating_world = TRUE;
    } else {
      return;
    }
  }

  yaw -= contData->stick_x / STICK_DAMPER;
  pitch += contData->stick_y / STICK_DAMPER;

  if (yaw < 0) {
    yaw += 360;
  } else if (yaw >= 360) {
    yaw -= 360;
  }

  if (pitch < 0) {
    pitch += 360;
  } else if (pitch >= 360) {
    pitch -= 360;
  }

  if (pitch > 90 && pitch < 180) {
    pitch = 90;
  }

  if (pitch < 270 && pitch > 180) {
    pitch = 270;
  }

  if(contData[0].button & U_CBUTTONS) {
    velocity.x -= sinf(yaw * M_DTOR) * MOVE_SPEED;
    velocity.z -= cosf(yaw * M_DTOR) * MOVE_SPEED;
  }

  if(contData[0].button & D_CBUTTONS) {
    velocity.x += sinf(yaw * M_DTOR) * MOVE_SPEED;
    velocity.z += cosf(yaw * M_DTOR) * MOVE_SPEED;
  }

  if(contData[0].button & L_CBUTTONS) {
    velocity.x -= cosf(yaw * M_DTOR) * MOVE_SPEED;
    velocity.z += sinf(yaw * M_DTOR) * MOVE_SPEED;
  }

  if(contData[0].button & R_CBUTTONS) {
    velocity.x += cosf(yaw * M_DTOR) * MOVE_SPEED;
    velocity.z -= sinf(yaw * M_DTOR) * MOVE_SPEED;
  }

  if(contData[0].button & START_BUTTON) {
    if (!block_dec_held) {
        block_dec_held = TRUE;

        held_block--;
        if (held_block < 1) {
          held_block = 7;
        }
    }
  } else {
    block_dec_held = FALSE;
  }

  if(contData[0].button & R_TRIG) {
    if (!block_inc_held) {
      block_inc_held = TRUE;

      held_block++;
      if (held_block > 7) {
        held_block = 1;
      }
    }
  } else {
    block_inc_held = FALSE;
  }

  if (onGround()) {
    if(contData[0].button & Z_TRIG) {
      y_velocity = JUMP_SPEED;
    } else {
      y_velocity = 0;
    }
  } else if (y_velocity > -TERMINAL_SPEED) {
    y_velocity -= GRAVITY;
  }
  velocity.y += y_velocity;

  while (t_total < 1) {
    t = detectCollision(velocity, 1 - t_total, &collision_axis);
    cam = add(cam, mul(velocity, t - 0.01));
    t_total += t;

    if (t_total < 1) {
      *at(&velocity, collision_axis) = 0;
      if (collision_axis == 1) {
        y_velocity = 0;
      }
    }
  }

  if (contData[0].trigger & A_BUTTON && target_present) {
    placeBlock(build_offset_x + target_x, build_offset_y + target_y, build_offset_z + target_z);
  }

  if (contData[0].trigger & B_BUTTON && target_present) {
    breakBlock(target_x, target_y, target_z);
  }
}