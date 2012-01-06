#include <iostream>

#include "gameboy.h"
#include "cpu.h"
#include "memory.h"
#include "debugger.h"

static byte colors[4][3] = {
    {196, 207, 161},
    {139, 149, 109},
    { 77,  83,  60},
    { 31,  31,  31},
};

GameBoy::GameBoy(const char *file) : buttons(0)
{
    memset(screen, 0, GB_DISPLAY_WIDTH * GB_DISPLAY_HEIGHT * 3);
    debugger = new Debugger();
    memory = new Memory(file, debugger);
    cpu = new CPU(memory, debugger);
}

GameBoy::~GameBoy()
{
    delete debugger;
    delete cpu;
    delete memory;
}

void GameBoy::setButton(Button btn, bool pressed)
{
    buttons = pressed ? (buttons | btn) : (buttons & ~btn);
}

void GameBoy::set_pixel(int x, int y, int color)
{
    if (x < 0 || x >= GB_DISPLAY_WIDTH || y < 0 || y >= GB_DISPLAY_HEIGHT || color > 3)
        return;
    screen[((y * GB_DISPLAY_WIDTH + x) * 3) + 0] = colors[color][0];
    screen[((y * GB_DISPLAY_WIDTH + x) * 3) + 1] = colors[color][1];
    screen[((y * GB_DISPLAY_WIDTH + x) * 3) + 2] = colors[color][2];
}

void GameBoy::fillScreen()
{
    memset(screen, 0, GB_DISPLAY_WIDTH * GB_DISPLAY_HEIGHT * 3);

    // Draw background
    for (int row = 0; row < 32; row++) {
        for (int col = 0; col < 32; col++) {
            byte tile = memory->get<byte>(0x9800 + (row * 32) + col);

            for (int y = 0; y < 8; y++) {
                int address = 0x8000 + (tile * 16) + (y * 2);
                byte byte1 = memory->get<byte>(address);
                byte byte2 = memory->get<byte>(address + 1);

                for (int x = 0; x < 8; x++) {
                    int i;
                    if (x == 0)
                        i = (byte1 & 1) + ((byte2 & 1) << 1);
                    else
                        i = ((byte1 & (1 << x)) >> (x))
                          + ((byte2 & (1 << x)) >> (x-1));
                    set_pixel((col * 8) + 7 - x, (row * 8) + y, i);
                }
            }
        }
    }

    // Draw sprites
    for (word sprite = 0xfe00; sprite <= 0xfe9f; sprite += 4) {
        byte ypos = memory->get<byte>(sprite);

        // Sprite hidden via ypos?
        if (ypos == 0 || ypos >= 160)
            continue;

        byte xpos = memory->get<byte>(sprite+word(1));

        // Sprite hidden via xpos?
        if (xpos == 0 || xpos >= 168)
            continue;

        //TODO: Ordering priority

        byte tile = memory->get<byte>(sprite+word(2));
        //byte attr = memory->get<byte>(sprite+word(3));

        for (int y = 0; y < 8; y++) {
            int address = 0x8000 + (tile * 16) + (y * 2);
            byte byte1 = memory->get<byte>(address);
            byte byte2 = memory->get<byte>(address + 1);

            for (int x = 0; x < 8; x++) {
                int i;
                if (x == 0)
                    i = (byte1 & 1) + ((byte2 & 1) << 1);
                else
                    i = ((byte1 & (1 << x)) >> (x))
                      + ((byte2 & (1 << x)) >> (x-1));
                set_pixel((xpos - 8) + 7 - x, (ypos - 16) + y, i);
            }
        }
    }
}

bool GameBoy::process()
{
    int oldCycles = cpu->cycles;
    while ((cpu->cycles - oldCycles) < 270) {
        // write joypad data
        byte b = memory->get<byte>(0xff00);
        if (~b & (1 << 5)) {
            b = (b & 0x00) | (0x0f & ~(0x0f & (buttons >> 4)));
        } else if (~b & (1 << 4)) {
            b = (b & 0x00) | (0x0f & ~(0x0f & buttons));
        } else {
            b |= 0x0f;
        }
        memory->set<byte>(0xff00, b);

        cpu->step();
    }

    cpu->ly += 1;
    if (cpu->ly > 153) {
        cpu->ly = 0;
    } else if (cpu->ly == 144) {
        /* vblank interrupt */
        cpu->requestInterrupt(INT_VBLANK);
        fillScreen();
        cpu->cycles = 0;

        return true;
    }

    return false;
}
