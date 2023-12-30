#ifndef GRAPHICS_H
#define GRAPHICS_H

#define SCREEN_HT 240
#define SCREEN_WD 320

#define DISPLAY_LIST_SIZE 65536
#define NUM_DISPLAY_LISTS 2

#define BLOCK_SIZE 64

extern Gfx* dlp;
Gfx display_lists[NUM_DISPLAY_LISTS][1024];
Gfx line_display_list[512];
Gfx hud_display_list[512];
extern u32 dl_no;

void initGraphics();
void makeWorldDisplayLists();
void makeDisplayListsAt(u8 x, u8 z);

#endif /* GRAPHICS_H */



