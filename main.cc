#include <iostream>

#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "gameboy.h"
#include "cpu.h"
#include "word.h"
#include "memory.h"
#include "debugger.h"

static GameBoy *gb = 0;
static GLubyte screen[GB_DISPLAY_WIDTH * GB_DISPLAY_HEIGHT * 3];
static int zoom = 2;

static bool key_right   = false;
static bool key_left    = false;
static bool key_up      = false;
static bool key_down    = false;
static bool key_a       = false;
static bool key_b       = false;
static bool key_select  = false;
static bool key_start   = false;

static GLubyte colors[4][3] = {
    {196, 207, 161},
    {139, 149, 109},
    { 77,  83,  60},
    { 31,  31,  31},
};

static void set_pixel(int x, int y, int color)
{
    if (x < 0 || x >= GB_DISPLAY_WIDTH || y < 0 || y >= GB_DISPLAY_HEIGHT || color > 3)
        return;
    screen[((y * GB_DISPLAY_WIDTH + x) * 3) + 0] = colors[color][0];
    screen[((y * GB_DISPLAY_WIDTH + x) * 3) + 1] = colors[color][1];
    screen[((y * GB_DISPLAY_WIDTH + x) * 3) + 2] = colors[color][2];
}

static void resize(int width, int height)
{
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
}

static void init()
{
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    memset(screen, 0, GB_DISPLAY_WIDTH * GB_DISPLAY_HEIGHT * 3);
}

static void cleanup()
{
    delete gb;
}

static void draw()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GB_DISPLAY_WIDTH, GB_DISPLAY_HEIGHT,
                 0, GL_RGB, GL_UNSIGNED_BYTE, screen);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, 0.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(1.0f, 0.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(1.0f, 1.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(0.0f, 1.0f);
    glEnd();

    glFlush();
}

static void fillScreen()
{
    memset(screen, 0, GB_DISPLAY_WIDTH * GB_DISPLAY_HEIGHT * 3);

    // Draw background
    for (int row = 0; row < 32; row++) {
        for (int col = 0; col < 32; col++) {
            byte tile = gb->memory->get<byte>(0x9800 + (row * 32) + col);

            for (int y = 0; y < 8; y++) {
                int address = 0x8000 + (tile * 16) + (y * 2);
                byte byte1 = gb->memory->get<byte>(address);
                byte byte2 = gb->memory->get<byte>(address + 1);

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
        byte ypos = gb->memory->get<byte>(sprite);

        // Sprite hidden via ypos?
        if (ypos == 0 || ypos >= 160)
            continue;

        byte xpos = gb->memory->get<byte>(sprite+word(1));

        // Sprite hidden via xpos?
        if (xpos == 0 || xpos >= 168)
            continue;

        //TODO: Ordering priority

        byte tile = gb->memory->get<byte>(sprite+word(2));
        //byte attr = gb->memory->get<byte>(sprite+word(3));

        for (int y = 0; y < 8; y++) {
            int address = 0x8000 + (tile * 16) + (y * 2);
            byte byte1 = gb->memory->get<byte>(address);
            byte byte2 = gb->memory->get<byte>(address + 1);

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

static void idle()
{
    int oldCycles = gb->cpu->cycles;
    while ((gb->cpu->cycles - oldCycles) < 270) {
        // write joypad data
        byte b = gb->memory->get<byte>(0xff00);
        if (~b & (1 << 5)) {
            b = (b & 0xf0) | ~((key_a << 0) | (key_b << 1) | (key_select << 2) | (key_start << 3));
        } else if (~b && (1 << 4)) {
            b = (b & 0xf0) | ~((key_right << 0) | (key_left << 1) | (key_up << 2) | (key_down << 3));
        } else {
            b |= 0x0f;
        }
        gb->memory->set<byte>(0xff00, b);

        gb->cpu->step();
    }

    gb->cpu->ly += 1;
    if (gb->cpu->ly > 153) {
        gb->cpu->ly = 0;
    } else if (gb->cpu->ly == 144) {
        /* vblank interrupt */
        gb->cpu->requestInterrupt(INT_VBLANK);
        fillScreen();
        glutPostRedisplay();
        gb->cpu->cycles = 0;
    }
}

void set_key(unsigned char key, bool down)
{
    switch (key) {
    case 'd': key_right = down; break;
    case 'a': key_left = down; break;
    case 'w': key_up = down; break;
    case 's': key_down = down; break;
    case 'o': key_a = down; break;
    case 'p': key_b = down; break;
    case 'u': key_select = down; break;
    case 'i': key_start = down; break;
    }
}

void keyDown(unsigned char key, int x, int y)
{
    set_key(key, true);
}

void keyUp(unsigned char key, int x, int y)
{
    set_key(key, false);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [sv] rom" << std::endl;
        return 1;
    }

    gb = new GameBoy(argv[argc-1]);
    if (!gb) {
        return 1;
    }
    atexit(cleanup);

    std::string options(argc > 2 ? argv[1] : "");
    gb->debugger->stepMode = options.find_first_of('s') != std::string::npos;
    gb->debugger->verboseCPU = options.find_first_of('v') != std::string::npos;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(GB_DISPLAY_WIDTH * zoom, GB_DISPLAY_HEIGHT * zoom);
    glutCreateWindow("gb");

    init();

    glutReshapeFunc(resize);
    glutDisplayFunc(draw);

    glutIdleFunc(idle);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);

    glutMainLoop();

    return 0;
}
