#include <nusys.h>
#include <assert.h>
#include "graphics.h"
#include "geometry.h"
#include "camera.h"
#include "player.h"
#include "menu.h"
#include "quads.h"
#include "cube.h"
#include "textures.h"

#define CROSSHAIR_SIZE 10

Gfx *dlp;
u32 dl_no = 0;

Gfx column_display_list[DISPLAY_LIST_SIZE];
Gfx *column_dlp;
Gfx *column_starts[NUM_TEXTURES][CHUNKS_X * CHUNKS_Z];

#define BLOCKS_PER_CHUNK (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

static Mtx c_models[NUM_BLOCKS / BLOCKS_PER_CHUNK];
static Mtx b_models[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

static Mtx marker_model;

static OSTime lt;

static u8 render_x;
static u8 render_y;
static u8 render_z;

static Texture *loaded_texture;

static Vp vp = {
    SCREEN_WD*2, SCREEN_HT*2, G_MAXZ/2, 0,
    SCREEN_WD*2, SCREEN_HT*2, G_MAXZ/2, 0,
};

static Gfx setup_display_list[] = {
  gsSPSegment(0, 0x0),
  gsSPViewport(&vp),
  gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
  gsDPSetScissor(G_SC_NON_INTERLACE, 0,0, SCREEN_WD,SCREEN_HT),
  gsSPEndDisplayList()
};

static Gfx draw_setup_display_list[] = {
  gsDPSetCycleType(G_CYC_1CYCLE),
  gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
  gsSPClearGeometryMode(0xFFFFFFFF),
  gsSPSetGeometryMode(G_ZBUFFER | G_CULL_BACK | G_SHADE | G_SHADING_SMOOTH),
  
  gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
  gsDPSetTexturePersp(G_TP_PERSP),
  gsDPSetCombineMode(G_CC_MODULATERGB, G_CC_MODULATERGB),
  gsDPSetTextureLUT(G_TT_RGBA16),
  gsSPEndDisplayList()
};

static Gfx crosshair_display_list[] = {
  gsDPSetCycleType(G_CYC_FILL),
  gsDPSetRenderMode (G_RM_NOOP, G_RM_NOOP2),
  gsDPSetFillColor((GPACK_RGBA5551(255, 255, 255, 1) << 16 | 
				GPACK_RGBA5551(255, 255, 255, 1))),
  gsDPFillRectangle((SCREEN_WD - CROSSHAIR_SIZE) / 2, SCREEN_HT / 2 - 1,
        (SCREEN_WD + CROSSHAIR_SIZE) / 2 - 1, SCREEN_HT / 2),
  gsDPFillRectangle(SCREEN_WD / 2 - 1, (SCREEN_HT - CROSSHAIR_SIZE) / 2,
        SCREEN_WD / 2, (SCREEN_HT + CROSSHAIR_SIZE) / 2 - 1),
  gsDPPipeSync(),
  gsDPSetCycleType(G_CYC_1CYCLE),
  gsDPSetRenderMode(G_RM_NOOP, G_RM_NOOP2),
  gsDPSetCombineMode(G_CC_MODULATEI_PRIM, G_CC_MODULATEI_PRIM),
  gsDPSetPrimColor(0,0,255,255,255,255),
  gsDPSetTexturePersp(G_TP_NONE),
  gsDPSetTextureLUT(G_TT_RGBA16),
  gsSPEndDisplayList()
};

static Gfx wireframe_setup_display_list[] = {
  gsDPSetCycleType(G_CYC_1CYCLE),
  gsDPSetRenderMode(G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2),
  gsSPClearGeometryMode(0xFFFFFFFF),
  gsSPSetGeometryMode(G_ZBUFFER | G_CULL_BACK | G_SHADE | G_SHADING_SMOOTH),
  gsSPEndDisplayList()
};

static Gfx wireframe_display_list[] = {
  gsSPLine3D(0, 1, 0),
  gsSPLine3D(1, 2, 0),
  gsSPLine3D(2, 3, 0),
  gsSPLine3D(3, 0, 0),
  
  gsSPLine3D(4, 5, 0),
  gsSPLine3D(5, 6, 0),
  gsSPLine3D(6, 7, 0),
  gsSPLine3D(7, 4, 0),
  
  gsSPLine3D(0, 5, 0),
  gsSPLine3D(1, 4, 0),
  gsSPLine3D(2, 7, 0),
  gsSPLine3D(3, 6, 0),

  gsSPEndDisplayList()
};

void clearBuffers(u16 bg_color) {
  gDPSetDepthImage(dlp++, OS_K0_TO_PHYSICAL(nuGfxZBuffer));
  gDPSetCycleType(dlp++, G_CYC_FILL);
  gDPSetColorImage(dlp++, G_IM_FMT_RGBA, G_IM_SIZ_16b,SCREEN_WD,
		   OS_K0_TO_PHYSICAL(nuGfxZBuffer));
  gDPSetFillColor(dlp++,(GPACK_ZDZ(G_MAXFBZ,0) << 16 |
			       GPACK_ZDZ(G_MAXFBZ,0)));
  gDPFillRectangle(dlp++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
  gDPPipeSync(dlp++);
  
  gDPSetColorImage(dlp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
		   osVirtualToPhysical(nuGfxCfb_ptr));
  gDPSetFillColor(dlp++, (bg_color << 16 | bg_color));
  gDPFillRectangle(dlp++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
  gDPPipeSync(dlp++);
}

void loadTexture(Texture *texture) {
  if (texture != loaded_texture) {
    loaded_texture = texture;
    gDPLoadTLUT_pal16(dlp++, 0, texture->pallet);
    gDPLoadTextureBlock_4b(dlp++, texture->color_indices, G_IM_FMT_CI,
                      16, 16, 0, G_TX_WRAP, loaded_texture == &grass_side_texture? G_TX_CLAMP : G_TX_WRAP, 
                      4, 4, G_TX_NOLOD, G_TX_NOLOD);
  }
}

void makeQuadDL(u8 chunk, u8 bx, u8 by, u8 bz, u8 width, u8 height, u8 face) {
  u32 b = bx * CHUNK_SIZE * CHUNK_SIZE + by * CHUNK_SIZE + bz;

  gSPMatrix(column_dlp++,OS_K0_TO_PHYSICAL(c_models + chunk),
    G_MTX_MODELVIEW|G_MTX_LOAD|G_MTX_NOPUSH);
  gSPMatrix(column_dlp++,OS_K0_TO_PHYSICAL(b_models + b),
    G_MTX_MODELVIEW|G_MTX_MUL|G_MTX_NOPUSH);
  gSPVertex(column_dlp++, QUAD_ADDR(face, width, height), 4, 0);
  gSP1Quadrangle(column_dlp++, 3, 2, 1, 0, 0);
}

void makeQuadDLRST(u8 chunk, u8 br, u8 bs, u8 bt, u8 axes, u8 width, u8 height, u8 face) {
  if (face == NONE) {
    return;
  }

  if (axes == ZXY) {
    makeQuadDL(chunk, bs, bt, br, width, height, face);
  } else if (axes == XZY) {
    makeQuadDL(chunk, br, bt, bs, width, height, face);
  } else if (axes == YXZ) {
    makeQuadDL(chunk, bs, br, bt, width, height, face);
  }
}

void makeChunkAxisDL(DualQuadList *axis_quads, u8 chunk, u8 axes, u8 face1, u8 face2, u8 block) {
  u8 br, i;
  DualQuadList *both_quads;
  for (br = 0; br < CHUNK_SIZE; br++) {
    both_quads = &(axis_quads)[br];
    for (i = 0; i < both_quads->n_front + both_quads->n_back; i++) {
      if (both_quads->quads[i].block == block) {
        makeQuadDLRST(chunk, br, both_quads->quads[i].bs, both_quads->quads[i].bt, axes,
          both_quads->quads[i].width, both_quads->quads[i].height, i < both_quads->n_front? face1 : face2);
      }
    }
  }
}

void makeColumnDL(u8 cx, u8 cz, u8 texture) {
  u8 cy, i, chunk;
  ChunkQuads *c_quads;
  FaceSpec *faces = textures[texture]->faces;

  column_starts[texture][cx * CHUNKS_Z + cz] = column_dlp;

  for (cy = 0; cy < CHUNKS_Y; cy++) {
    chunk = cx * CHUNKS_Y * CHUNKS_Z + cy * CHUNKS_Z + cz;
    c_quads = &chunk_quads[chunk];

    for (i = 0; i < textures[texture]->n_faces; i++) {
      if (faces[i].sides) {
        makeChunkAxisDL(c_quads->z_quads, chunk, ZXY, FRONT, BACK, faces[i].block);
        makeChunkAxisDL(c_quads->x_quads, chunk, XZY, RIGHT, LEFT, faces[i].block);
      }

      if (faces[i].top || faces[i].bottom) {
        makeChunkAxisDL(c_quads->y_quads, chunk, YXZ, faces[i].top? TOP : NONE, faces[i].bottom? BOTTOM : NONE, faces[i].block);
      }
    }
  }

  gSPEndDisplayList(column_dlp++);
}

void makeWorldDisplayLists() {
  u8 cx, cz, i;
  for (i = 0; i < NUM_TEXTURES; i++) {
    for (cx = 0; cx < CHUNKS_X; cx++) {
      for (cz = 0; cz < CHUNKS_Z; cz++) {
        makeColumnDL(cx, cz, i);
      }
    }
  }
}

void makeDisplayListsAt(u8 x, u8 z) {
  u8 i;
  u8 cx = x / CHUNK_SIZE;
  u8 cz = z / CHUNK_SIZE;

  if ((column_display_list + DISPLAY_LIST_SIZE - column_dlp) < (DISPLAY_LIST_SIZE) / 10) {
    column_dlp = column_display_list;
    makeWorldDisplayLists();
  } else {
    for (i = 0; i < NUM_TEXTURES; i++) {
      makeColumnDL(cx, cz, i);
      if (x % CHUNK_SIZE == 0 && cx > 0) {
        makeColumnDL(cx - 1, cz, i);
      }
      if (z % CHUNK_SIZE == 0 && cz > 0) {
        makeColumnDL(cx, cz - 1, i);
      }
    }
  }
}

void drawTextured(u8 texture) {
  u8 cx, cz;
  for (cx = 0; cx < CHUNKS_X; cx++) {
    for (cz = 0; cz < CHUNKS_Z; cz++) {
      if (visible_columns[cx * CHUNKS_Z + cz]) {
        gSPDisplayList(dlp++, column_starts[texture][cx * CHUNKS_Z + cz]);
      }
    }
  }
}

void drawWorld() {
  u8 i;

  dlp = &display_lists[dl_no][0];

  gSPDisplayList(dlp++, setup_display_list);

  clearBuffers(GPACK_RGBA5551(158, 207, 255, 1));

  gSPDisplayList(dlp++, draw_setup_display_list);

  loadCameraMatrices();

  for (i = 0; i < NUM_TEXTURES; i++) {
    loadTexture(textures[i]->texture);
    drawTextured(i);
  }

  gDPFullSync(dlp++);
  gSPEndDisplayList(dlp++);

  nuGfxTaskStart(&display_lists[dl_no][0],
		(s32)(dlp - display_lists[dl_no]) * sizeof (Gfx),
		NU_GFX_UCODE_F3DEX,
    NU_SC_NOSWAPBUFFER
  );
}

void drawWireframe() {
  dlp = line_display_list;

  gSPDisplayList(dlp++, setup_display_list);

  gSPDisplayList(dlp++, wireframe_setup_display_list);

  loadCameraMatrices();
  
  gSPMatrix(dlp++,OS_K0_TO_PHYSICAL(c_models + (target_x / CHUNK_SIZE) * CHUNKS_Y * CHUNKS_Z + (target_y / CHUNK_SIZE) * CHUNKS_Z + (target_z / CHUNK_SIZE)),
    G_MTX_MODELVIEW|G_MTX_LOAD|G_MTX_NOPUSH);
  gSPMatrix(dlp++,OS_K0_TO_PHYSICAL(b_models + (target_x % CHUNK_SIZE) * CHUNK_SIZE * CHUNK_SIZE + (target_y % CHUNK_SIZE) * CHUNK_SIZE + (target_z % CHUNK_SIZE)),
    G_MTX_MODELVIEW|G_MTX_MUL|G_MTX_NOPUSH);
  gSPVertex(dlp++, cube_verts, 8, 0);

  gSPDisplayList(dlp++, wireframe_display_list);

  gDPFullSync(dlp++);
  gSPEndDisplayList(dlp++);

  nuGfxTaskStart(line_display_list,
		(s32)(dlp - line_display_list) * sizeof (Gfx),
		NU_GFX_UCODE_L3DEX,
    NU_SC_NOSWAPBUFFER
  );
}

void drawHUD() {
  dlp = hud_display_list;

  gSPDisplayList(dlp++, setup_display_list);

  if (current_screen != GAME) {
    clearBuffers(GPACK_RGBA5551(0, 0, 0, 1));
  } else {
    gSPDisplayList(dlp++, crosshair_display_list);

    loadTexture(preview_textures[held_block]);
    gSPTextureRectangle(dlp++,
      10 << 2, 10 << 2,
      (26 << 2) - 2, (26 << 2) - 2,
      G_TX_RENDERTILE,
      0, 0,
      1 << 10, 1 << 10);
    gDPPipeSync(dlp++);
  }
  drawMenu();

  gDPFullSync(dlp++);
  gSPEndDisplayList(dlp++);

  nuGfxTaskStart(hud_display_list,
		(s32)(dlp - hud_display_list) * sizeof (Gfx),
		NU_GFX_UCODE_S2DEX,
    osTvType == OS_TV_NTSC? NU_SC_NOSWAPBUFFER : NU_SC_SWAPBUFFER
  );
}

void draw() {
  char conbuf[20];
  OSTime t;
  loaded_texture = NULL;

  if (current_screen == GAME) {
    updateVisibleColumns();
    updateCameraMatrices();

    drawWorld();
    if (target_present) {
      drawWireframe();
    }
  }
  drawHUD();

  if (osTvType == OS_TV_NTSC) {
    // nuDebTaskPerfBar0EX2(4, 20, NU_SC_NOSWAPBUFFER);
    // t = osGetTime();

    // nuDebConTextPos(0,0,0);
    // sprintf(conbuf,"%llu", 1000000 / OS_CYCLES_TO_USEC(t - lt));
    // nuDebConCPuts(0, conbuf);

    // lt = t;
      
    nuDebConDisp(NU_SC_SWAPBUFFER);
  }

  /* Switch display list buffers */
  dl_no ^= 1;
}

void initGraphics() {
  int x, y, z, i;

  render_x = 0;
  render_y = 0;
  render_z = 0;

  nuGfxInit();
  nuGfxDisplayOn();

  for (x = 0; x < CHUNKS_X; x++) {
    for (y = 0; y < CHUNKS_Y; y++) {
      for (z = 0; z < CHUNKS_Z; z++) {
        guTranslate(&(c_models[x * CHUNKS_Y * CHUNKS_Z + y * CHUNKS_Z + z]), x * BLOCK_SIZE * CHUNK_SIZE, y * BLOCK_SIZE * CHUNK_SIZE, z * BLOCK_SIZE * CHUNK_SIZE);
      }
    }
  }

  for (x = 0; x < CHUNK_SIZE; x++) {
    for (y = 0; y < CHUNK_SIZE; y++) {
      for (z = 0; z < CHUNK_SIZE; z++) {
        guTranslate(&(b_models[x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z]), x * BLOCK_SIZE, y * BLOCK_SIZE, z * BLOCK_SIZE);
      }
    }
  }

  column_dlp = column_display_list;
}
