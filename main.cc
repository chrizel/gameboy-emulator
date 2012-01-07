#include <iostream>
#include <string>
#include <cstdlib>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#ifndef GL_CLAMP_TO_EDGE // Not defined in Microsofts ancient headers
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#include "gameboy.h"
#include "debugger.h"

static GameBoy *gb = 0;
static int zoom = 2;

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
                 0, GL_RGB, GL_UNSIGNED_BYTE, gb->getScreen());
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

static void idle()
{
    if (gb->process())
        glutPostRedisplay();
}

static void setKey(unsigned char key, bool down)
{
    switch (key) {
    case 'd': gb->setButton(BTN_RIGHT,  down); break;
    case 'a': gb->setButton(BTN_LEFT,   down); break;
    case 'w': gb->setButton(BTN_UP,     down); break;
    case 's': gb->setButton(BTN_DOWN,   down); break;
    case 'o': gb->setButton(BTN_A,      down); break;
    case 'p': gb->setButton(BTN_B,      down); break;
    case 'u': gb->setButton(BTN_SELECT, down); break;
    case 'i': gb->setButton(BTN_START,  down); break;
    case 'v': if (!down) { gb->getDebugger()->verboseCPU = !gb->getDebugger()->verboseCPU; } break;
    case 'b': if (!down) { gb->getDebugger()->stepMode = true; } break;
    }
}

static void keyDown(unsigned char key, int x, int y)
{
    setKey(key, true);
}

static void keyUp(unsigned char key, int x, int y)
{
    setKey(key, false);
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
    gb->getDebugger()->stepMode = options.find_first_of('s') != std::string::npos;
    gb->getDebugger()->verboseCPU = options.find_first_of('v') != std::string::npos;

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
