#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gameboy.h"
#include "cpu.h"

GameBoy::GameBoy(const char *file)
{
    int size;
    FILE *fp;

    fp = fopen(file, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open file: %s\n", file);
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    mem = (byte*)malloc(0xFFFF);
    fread(mem, 1, size, fp);

    fclose(fp);

    gbCPUInit(this);
}

GameBoy::~GameBoy()
{
    free(mem);
}

void GameBoy::process()
{
    cpu.cycles = 0;
    while (cpu.cycles < 100) {
        gbCPUStep(this);

        REG_LY(this)++;
        if (REG_LY(this) > 153) {
            REG_LY(this) = 0;
        } else if (REG_LY(this) == 144) {
            /* vblank interrupt */
        }
    }
}
