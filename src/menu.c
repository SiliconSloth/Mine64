#include "menu.h"
#include "graphics.h"
#include "font.h"
#include "storage.h"

enum Screen current_screen = MENU;
u32 save_message_cooldown = 0;

static char *menu_text[] = {
  "Mine64",
  "",
  "",
  "How to play",
  "",
  "New world",
  "New world",
  "New world"
};

static char *menu_text_no_saving[] = {
  "Mine64",
  "",
  "",
  "How to play",
  "",
  "New world",
  "",
  "",
  "WARNING",
  "CANNOT SAVE GAME",
  "ON THIS SYSTEM"
};

static char *world_names[] = {
  "World 1",
  "World 2",
  "World 3"
};

static u8 option_lines[] = {
  3, 5, 6, 7
};

static u8 selected_option = 0;

static char *info_text[] = {
  "Mine64",
  "",
  "Look around: Analog stick",
  "Walk: C buttons",
  "Place block: A button",
  "Break block: B button",
  "Select block: Start/Right shoulder",
  "Jump: Z trigger",
  "Save game: D-Pad"
};

static char *generating_text[] = {
  "Generating world..."
};

static char *loading_text[] = {
  "Loading world..."
};

static char *saved_text[] = {
  "World saved"
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
  u8 option_y;
  char **text;
  char *text_line;
  u32 n_lines;
  u32 y_start = SCREEN_HT / 6;

  switch (current_screen) {
    case MENU:
      text = saving_available? menu_text : menu_text_no_saving;
      n_lines = saving_available? sizeof(menu_text) / sizeof(char *) : sizeof(menu_text_no_saving) / sizeof(char *);
      break;
    case INFO:
      text = info_text;
      n_lines = sizeof(info_text) / sizeof(char *);
      break;
    case GENERATING:
      text = generating_text;
      n_lines = sizeof(generating_text) / sizeof(char *);
      y_start = SCREEN_HT / 3;
      break;
    case LOADING:
      text = loading_text;
      n_lines = sizeof(loading_text) / sizeof(char *);
      y_start = SCREEN_HT / 3;
      break;
    case GAME:
      text = saved_text;
      n_lines = sizeof(saved_text) / sizeof(char *);
      y_start = SCREEN_HT / 3;
      break;
  }

  if (current_screen == GAME && save_message_cooldown == 0) {
    return;
  }

  if (save_message_cooldown > 0) {
    save_message_cooldown--;
  }

  gSPDisplayList(dlp++, menu_setup_display_list);

  for (i = 0; i < n_lines; i++) {
    if (current_screen == MENU && i >= option_lines[1] && files_present[i - option_lines[1]]) {
      text_line = world_names[i - option_lines[1]];
    } else {
      text_line = text[i];
    }

    j = 0;
    center = 0;
    while (text_line[j]) {
      chr = text_line[j];
      center += charWidth(chr);

      if (chr == ':') {
        break;
      }

      j++;
    }

    if (text_line[j]) {
      center += charWidth(' ') / 2;
    } else {
      center /= 2;
    }

    j = 0;
    x = 0;
    while (text_line[j]) {
      chr = text_line[j];
      if (chr != ' ') {
        drawChar(chr, x + SCREEN_WD / 2 - center, i * 12 + y_start);
      }

      x += charWidth(chr);
      j++;
    }
  }
  
  if (current_screen == MENU) {
    option_y = option_lines[selected_option] * 12 + y_start;
    drawChar('>', SCREEN_WD / 2 - 40 - charWidth('>'), option_y);
    drawChar('<', SCREEN_WD / 2 + 40, option_y);
  }
}

void menuDown() {
  if (current_screen == MENU) {
    if (selected_option == (saving_available? 3 : 1)) {
      selected_option = 0;
    } else {
      selected_option++;
    }
  }
}

void menuUp() {
  if (current_screen == MENU) {
    if (selected_option == 0) {
      selected_option = saving_available? 3 : 1;
    } else {
      selected_option--;
    }
  }
}

void menuAct() {
  if (current_screen == MENU) {
    if (selected_option == 0) {
      current_screen = INFO;
    } else{
      game_file_num = selected_option;
      if (files_present[selected_option - 1]) {
        current_screen = LOADING;
      } else {
        current_screen = GENERATING;
      }
    }
  } else if (current_screen == INFO) {
    current_screen = MENU;
  }
}