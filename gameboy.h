#ifndef GAMEBOY_H
#define GAMEBOY_H

typedef struct {
    char *mem;
} GameBoy;

GameBoy * gbInit(const char *file);
void gbFree(GameBoy *gb);

#define GB_DISPLAY_WIDTH 160
#define GB_DISPLAY_HEIGHT 144

#endif
