#ifndef CAMERA_H
#define CAMERA_H

#include <nusys.h>
#include "geometry.h"

u8 visible_columns[CHUNKS_X * CHUNKS_Z];

void initCamera();
void updateVisibleColumns();
void updateCameraMatrices();
void loadCameraMatrices();

#endif /* CAMERA_H */