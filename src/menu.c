#include "menu.h"
#include "main.h"
#include "graphics.h"
#include "font.h"

static char *info_text[] = {
  "Mine64",
  "",
  "Look around: Analog stick",
  "Walk: C buttons",
  "Place block: A button",
  "Break block: B button",
  "Select block: Start/Right shoulder",
  "Jump: Z trigger",
  "",
  "Press to start"
};

static char *generating_text[] = {
  "Generating world..."
};

static Gfx menu_setup_display_list[] = {
  gsDPSetCycleType(G_CYC_1CYCLE),
  gsDPSetRenderMode(G_RM_NOOP, G_RM_NOOP2),
  gsDPSetCombineMode(G_CC_MODULATEI_PRIM, G_CC_MODULATEI_PRIM),
  gsDPSetPrimColor(0,0,255,255,255,255),
  gsDPSetTexturePersp(G_TP_NONE),
  gsDPSetTextureLUT(G_TT_NONE),
  gsDPLoadTextureTile_4b(font_texture, G_IM_FMT_I, 128, 64,
        0, 0, 32 << 2, 16 << 2,
        0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
        G_TX_NOLOD, G_TX_NOLOD),
  gsSPEndDisplayList()
};

u32 charWidth(char chr) {
  if (chr == 'i' || chr == ':' || chr == '.' || chr == ' ') {
    return 3;
  } else if (chr == 'l') {
    return 4;
  } else if (chr == 't') {
    return 5;
  } else if (chr == 'k') {
    return 6;
  } else {
    return 7;
  }
}

void drawChar(char chr, u32 x, u32 y) {
  u8 idx = chr - ' ';
  u32 cx = idx % 16;
  u32 cy = (idx / 16) + 2;

  gSPTextureRectangle(dlp++,
    x << 2, y << 2,
    ((x + 8) << 2) - 2, ((y + 8) << 2) - 2,
    G_TX_RENDERTILE,
    (cx * 8) << 5, (cy * 8) << 5,
    1 << 10, 1 << 10);
}

void drawMenu() {
  u32 i, j, x, center;
  char chr;

  char **text = generating_world? generating_text : info_text;
  u32 n_lines = (generating_world? sizeof(generating_text) : sizeof(info_text)) / sizeof(char *);

  u32 y_start = SCREEN_HT / (generating_world? 3 : 6);

  gSPDisplayList(dlp++, menu_setup_display_list);

  for (i = 0; i < n_lines; i++) {
    j = 0;
    center = 0;
    while (text[i][j]) {
      chr = text[i][j];
      center += charWidth(chr);

      if (chr == ':') {
        break;
      }

      j++;
    }

    if (text[i][j]) {
      center += charWidth(' ') / 2;
    } else {
      center /= 2;
    }

    j = 0;
    x = 0;
    while (text[i][j]) {
      chr = text[i][j];
      if (chr != ' ') {
        drawChar(chr, x + SCREEN_WD / 2 - center, i * 12 + y_start);
      }

      x += charWidth(chr);
      j++;
    }
  }
}