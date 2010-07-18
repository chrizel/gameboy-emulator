SOURCES=main.cc gameboy.cc cpu.cc memory.cc debugger.cc
HEADERS=gameboy.h cpu.h memory.h debugger.h
gb: $(SOURCES) $(HEADERS)
	clang++ -Wall -framework GLUT -framework OpenGL -framework Cocoa $(SOURCES) -o gb
.PHONY: debug

clean:
	rm -rf gb
