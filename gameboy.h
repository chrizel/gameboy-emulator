#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "word.h"

const byte GB_DISPLAY_WIDTH  = 160;
const byte GB_DISPLAY_HEIGHT = 144;

enum Button
{
    BTN_RIGHT  = 1 << 0,
    BTN_LEFT   = 1 << 1,
    BTN_UP     = 1 << 2,
    BTN_DOWN   = 1 << 3,
    BTN_A      = 1 << 4,
    BTN_B      = 1 << 5,
    BTN_SELECT = 1 << 6,
    BTN_START  = 1 << 7
};

class Memory;
class CPU;
class Debugger;

class GameBoy
{
private:
    byte buttons;
    byte screen[GB_DISPLAY_WIDTH * GB_DISPLAY_HEIGHT * 3];
    Debugger *debugger;
    Memory *memory;
    CPU *cpu;

    void set_pixel(int x, int y, int color);
    void fillScreen();
public:

    GameBoy(const char *file);
    virtual ~GameBoy();

    bool process();
    void setButton(Button btn, bool pressed);

    const byte *getScreen() const { return screen; }

    Debugger *getDebugger() { return debugger; }
};

#endif
