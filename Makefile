SOURCES=main.cc gameboy.cc cpu.cc debugger.cc
HEADERS=gameboy.h cpu.h debugger.h
gb: $(SOURCES) $(HEADERS)
	g++ -Wall -framework GLUT -framework OpenGL -framework Cocoa $(SOURCES) -o gb
.PHONY: debug

clean:
	rm -rf gb
