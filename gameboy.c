#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gameboy.h"
#include "cpu.h"

GameBoy * gbGameBoyCreate(const char *file)
{
    int size;
    GameBoy *gb;
    FILE *fp;

    fp = fopen(file, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open file: %s\n", file);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    gb = malloc(sizeof(GameBoy));
    gb->mem = malloc(0xFFFF);
    fread(gb->mem, 1, size, fp);

    fclose(fp);

    gbCPUInit(gb);

    return gb;
}

void gbGameBoyProcess(GameBoy *gb)
{
    gb->cpu.cycles = 0;
    while (gb->cpu.cycles < 100) {
        gbCPUStep(gb);

        REG_LY(gb)++;
        if (REG_LY(gb) > 153) {
            REG_LY(gb) = 0;
        } else if (REG_LY(gb) == 144) {
            /* vblank interrupt */
        }
    }
}

void gbGameBoyFree(GameBoy *gb)
{
    if (gb) {
        free(gb->mem);
        free(gb);
    }
}
