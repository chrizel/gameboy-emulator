#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "gameboy.h"

static GameBoy *gb = NULL;
static GLubyte screen[GB_DISPLAY_WIDTH * GB_DISPLAY_HEIGHT * 3];
static int zoom = 2;

static GLubyte colors[4][3] = {
    {196, 207, 161},
    {139, 149, 109},
    { 77,  83,  60},
    { 31,  31,  31},
};

static void set_pixel(int x, int y, int color)
{
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
    gbFree(gb);
}

static void draw()
{
    int x, y;

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    for (y = 0; y < GB_DISPLAY_HEIGHT; y++) {
        for (x = 0; x < GB_DISPLAY_WIDTH; x++) {
            set_pixel(x, y, 0);
        }
    }

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

static void idle()
{
    glutPostRedisplay();
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s rom\n", argv[0]);
        return 1;
    }
    gb = gbInit(argv[1]);
    if (!gb) {
        return 1;
    }
    atexit(cleanup);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(GB_DISPLAY_WIDTH * zoom, GB_DISPLAY_HEIGHT * zoom);
    glutCreateWindow("gb");

    init();

    glutReshapeFunc(resize);
    glutDisplayFunc(draw);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}
