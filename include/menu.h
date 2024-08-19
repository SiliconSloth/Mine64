#ifndef MENU_H
#define MENU_H

#include <nusys.h>

enum Screen {
  MENU,
  INFO,
  GENERATING,
  LOADING,
  GAME
};

extern enum Screen current_screen;
extern u32 save_message_cooldown;

void drawMenu();
void menuDown();
void menuUp();
void menuAct();

#endif /* MENU_H */