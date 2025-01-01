#include <nusys.h>
#include "blocks.h"
#include "texture_data.h"

typedef struct {
  u8 block;
  u8 top;
  u8 bottom;
  u8 sides;
} FaceSpec;

FaceSpec dirt_faces[] = {
  {DIRT,  TRUE, TRUE, TRUE},
  {GRASS, FALSE, TRUE, FALSE}
};

FaceSpec stone_faces[] = {
  {STONE, TRUE, TRUE, TRUE}
};

FaceSpec grass_top_faces[] = {
  {GRASS, TRUE, FALSE, FALSE}
};

FaceSpec grass_side_faces[] = {
  {GRASS, FALSE, FALSE, TRUE}
};

FaceSpec cobblestone_faces[] = {
  {COBBLESTONE, TRUE, TRUE, TRUE}
};

FaceSpec sand_faces[] = {
  {SAND, TRUE, TRUE, TRUE}
};

FaceSpec wood_top_faces[] = {
  {WOOD, TRUE, TRUE, FALSE}
};

FaceSpec wood_side_faces[] = {
  {WOOD, FALSE, FALSE, TRUE}
};

FaceSpec leaves_faces[] = {
  {LEAVES, TRUE, TRUE, TRUE}
};

FaceSpec planks_faces[] = {
  {PLANKS, TRUE, TRUE, TRUE}
};

FaceSpec bricks_faces[] = {
  {BRICKS, TRUE, TRUE, TRUE}
};

typedef struct {
  Texture *texture;
  u8 n_faces;
  FaceSpec *faces;
} TextureSpec;

TextureSpec dirt_spec = {
  &dirt_texture, 2, dirt_faces
};

TextureSpec stone_spec = {
  &stone_texture, 1, stone_faces
};

TextureSpec grass_top_spec = {
  &grass_top_texture, 1, grass_top_faces
};

TextureSpec grass_side_spec = {
  &grass_side_texture, 1, grass_side_faces
};

TextureSpec cobblestone_spec = {
  &cobblestone_texture, 1, cobblestone_faces
};

TextureSpec sand_spec = {
  &sand_texture, 1, sand_faces
};

TextureSpec wood_top_spec = {
  &wood_top_texture, 1, wood_top_faces
};

TextureSpec wood_side_spec = {
  &wood_side_texture, 1, wood_side_faces
};

TextureSpec leaves_spec = {
  &leaves_texture, 1, leaves_faces
};

TextureSpec planks_spec = {
  &planks_texture, 1, planks_faces
};

TextureSpec bricks_spec = {
  &bricks_texture, 1, bricks_faces
};

TextureSpec *textures[] = {
  &dirt_spec, &stone_spec, &grass_top_spec, &grass_side_spec, &cobblestone_spec, &sand_spec,
  &wood_top_spec, &wood_side_spec, &leaves_spec, &planks_spec, &bricks_spec
};

Texture *preview_textures[] = {
  NULL, &dirt_texture, &stone_texture, &grass_side_texture,
  &cobblestone_texture, &sand_texture, &wood_side_texture,
  &leaves_texture, &planks_texture, &bricks_texture
};

#define NUM_TEXTURES (sizeof(textures) / sizeof(textures[0]))
