SOURCES=main.c gameboy.c cpu.c debug.c
HEADERS=gameboy.h cpu.h debug.c
gb: $(SOURCES) $(HEADERS)
	gcc -Wall -framework GLUT -framework OpenGL -framework Cocoa $(SOURCES) -o gb
.PHONY: debug

clean:
	rm -rf gb
