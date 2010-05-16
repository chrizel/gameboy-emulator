gb: main.c gameboy.c gameboy.h
	gcc -Wall -framework GLUT -framework OpenGL -framework Cocoa main.c gameboy.c -o gb
