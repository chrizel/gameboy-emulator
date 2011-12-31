SOURCES=main.cc gameboy.cc cpu.cc memory.cc debugger.cc
HEADERS=word.h gameboy.h cpu.h memory.h debugger.h
PROG=gb
$(PROG): $(SOURCES) $(HEADERS)
	clang++ -Wall -framework GLUT -framework OpenGL -framework Cocoa $(SOURCES) -o $(PROG)
.PHONY: run clean

clean:
	rm -rf $(PROG)
