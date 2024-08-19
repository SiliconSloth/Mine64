#ifndef STORAGE_H
#define STORAGE_H

#include <nusys.h>

extern u8 saving_available;
extern u8 files_present[3];
extern u32 game_file_num;

void initStorage();
void saveGame();
void loadGame();

#endif /* STORAGE_H */