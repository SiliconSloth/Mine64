#ifndef PLAYER_H
#define PLAYER_H

#include <nusys.h>
#include "math.h"

float pitch;
float yaw;

Vector3 cam;

int held_block;

u8 target_x;
u8 target_y;
u8 target_z;

s8 build_offset_x;
s8 build_offset_y;
s8 build_offset_z;

u8 target_present;

void initPlayer();
void updateTargetBlock();
void updatePlayer();

#endif /* PLAYER_H */