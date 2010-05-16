#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gameboy.h"

GameBoy * gbInit(const char *file)
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
    printf("Rom size: %d bytes\n", size);

    gb = malloc(sizeof(GameBoy));
    gb->mem = malloc(size);
    fread(gb->mem, 1, size, fp);

    fclose(fp);

    return gb;
}

void gbFree(GameBoy *gb)
{
    if (gb) {
        free(gb->mem);
        free(gb);
    }
}
