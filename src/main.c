#include <nusys.h>
#include "main.h"
#include "menu.h"
#include "camera.h"
#include "player.h"
#include "geometry.h"
#include "graphics.h"
#include "storage.h"

void draw(void);

void callbackGfx(int pendingGfx) {
  updateTargetBlock();
  
  if(pendingGfx < 2) {
    draw();
  }

  if (current_screen == GENERATING) {
    initWorld();
    initPlayer();
    initGeometry();
    makeWorldDisplayLists();

    current_screen = GAME;
  }

  if (current_screen == LOADING) {
    loadGame();
    initGeometry();
    makeWorldDisplayLists();
    
    current_screen = GAME;
  }

  updatePlayer();
}

void callbackPreNMI() {
    nuGfxDisplayOff();
    osViSetYScale(1);
    osAfterPreNMI();
}

void initVideo() {
  osCreateViManager(OS_PRIORITY_VIMGR);

  if (osTvType == OS_TV_NTSC) {
    osViSetMode(&osViModeNtscLan1);
  } else if (osTvType == OS_TV_PAL) {
    osViSetMode(&osViModeFpalLan1);
    osViSetYScale(0.833);
  } else if (osTvType == OS_TV_MPAL) {
    osViSetMode(&osViModeMpalLan1);
  }

  osViSetSpecialFeatures(OS_VI_GAMMA_OFF);
  nuPreNMIFuncSet((NUScPreNMIFunc) callbackPreNMI);
}

void mainproc(void) {
  initVideo();
  initCamera();
  initGraphics();
  initStorage();
  
  nuContInit();
  nuGfxFuncSet((NUGfxFunc) callbackGfx);

  while(1)
    ;
}
