#include <algorithm>
#include <iomanip>
#include <string>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "word.h"
#include "cpu.h"
#include "debugger.h"
#include "memory.h"
#include "instructions.h"

static const bool CONSOLE_COLORS = false;
static const std::string CONSOLE_RED   = CONSOLE_COLORS ? "\x1b[31m" : "";
static const std::string CONSOLE_GREEN = CONSOLE_COLORS ? "\x1b[32m" : "";
static const std::string CONSOLE_BLUE  = CONSOLE_COLORS ? "\x1b[34m" : "";
static const std::string CONSOLE_RESET = CONSOLE_COLORS ? "\x1b[0m"  : "";

Debugger::Debugger() : verboseCPU(false), verboseMemory(false), stepMode(true)
{
}

void Debugger::toggleBreakpoint(word address)
{
    Breakpoints::iterator it = std::find(breakpoints.begin(), breakpoints.end(), address);
    if(it == breakpoints.end()) {
        std::cout << "Set breakpoint " << address << std::endl;
        breakpoints.push_back(address);
    } else {
        std::cout << "Remove breakpoint " << address << std::endl;
        breakpoints.erase(it);
    }
}

void Debugger::listBreakpoints()
{
    if (breakpoints.empty()) {
        std::cout << "No breakpoints set" << std::endl;
    } else {
        std::cout << "List breakpoints:" << std::endl;
        for (Breakpoints::iterator it = breakpoints.begin(); it != breakpoints.end(); ++it) {
            std::cout << '\t' << *it << std::endl;
        }
    }
}

void Debugger::toggleWatch(word address)
{
    Watches::iterator it = std::find(watches.begin(), watches.end(), address);
    if(it == watches.end()) {
        std::cout << "Set watch " << address << std::endl;
        watches.push_back(address);
    } else {
        std::cout << "Remove watch " << address << std::endl;
        watches.erase(it);
    }
}

void Debugger::listWatches()
{
    if (watches.empty()) {
        std::cout << "No watches set" << std::endl;
    } else {
        std::cout << "List watches:" << std::endl;
        for (Watches::iterator it = watches.begin(); it != watches.end(); ++it) {
            std::cout << '\t' << *it << std::endl;
        }
    }
}

void Debugger::handleInstruction(CPU *cpu, word address)
{
    if (stepMode) {
        printInstruction(cpu, address);
        prompt(cpu);
    } else if(std::find(breakpoints.begin(), breakpoints.end(), address) != breakpoints.end()) {
        std::cout << "Breakpoint at" << std::endl;
        printInstruction(cpu, address);
        prompt(cpu);
    } else if (verboseCPU) {
        printInstruction(cpu, address);
    }
}

void Debugger::handleMemoryAccess(Memory *memory, word address, bool set)
{
    static bool inHandleMemoryAccess = false;
    if (inHandleMemoryAccess || (!verboseMemory && watches.empty()))
        return;

    inHandleMemoryAccess = true;
    Watches::iterator it = std::find(watches.begin(), watches.end(), address);
    if(verboseMemory || (it != watches.end())) {
        if (set) {
            std::cout << CONSOLE_RED << " set " << address << " to " 
                      << memory->get<byte>(address) << CONSOLE_RESET << std::endl;
        } else {
            std::cout << CONSOLE_GREEN << " mget " << address << " -> " 
                      << memory->get<byte>(address) << CONSOLE_RESET << std::endl;
        }
    }
    inHandleMemoryAccess = false;
}

void Debugger::handleInterrupt(int irq, word address)
{
    if (!verboseCPU)
        return;

    std::cout << CONSOLE_BLUE << "interrupt " << irq << " (" << address << ")" << CONSOLE_RESET << std::endl;
}

void Debugger::showMemory(CPU *cpu, word address)
{
    const byte colcount = 8;
    const byte rowcount = 16;

    word base;
    if (address.value() < (colcount*rowcount/2))
        base = (colcount*rowcount/2);
    else
        base = address;
    word start = base - (base.value() % colcount) - (colcount*(rowcount/2));
    word end = start + word(colcount*rowcount)-1;
    if (end < start)
        end = 0xffff;

    word i = start;
    while (true) {
        if (i.value() % colcount == 0) {
            std::cout << std::endl << '\t' << i;
        }

        if (i == address) {
            std::cout << " " << CONSOLE_RED << cpu->memory->get<byte>(i) << CONSOLE_RESET;
        } else {
            std::cout << " " << cpu->memory->get<byte>(i);
        }
        if (i == end)
            break;
        i++;
    }
    std::cout << std::endl;
}

void Debugger::showStack(CPU *cpu)
{
    word start = cpu->sp + word(8);
    if (start < 0x0010)
        start = 0xffff;
    for (word i = start; i >= cpu->sp; i--) {
        if (i == cpu->sp)
            std::cout << "\t" << CONSOLE_RED << i << " " << cpu->memory->get<byte>(i) << CONSOLE_RESET << std::endl;
        else
            std::cout << "\t" << i << " " << cpu->memory->get<byte>(i) << std::endl;
    }
}

void Debugger::printInstruction(CPU *cpu, word address)
{
    word i = 0;
    Instruction * instruction = cpu->findInstruction(address);
    if (instruction) {
        std::cout << "\t" << address << "\t";
        for (i = 0; i < 3; i++) {
            if (i < instruction->length) {
                std::cout << cpu->memory->get<byte>(address+i) << " ";
            } else {
                std::cout << "    ";
            }
        }
        std::cout << "\t" << instruction->mnemonic << std::endl;
        return;
    }

    std::cout << "\t" << address << "\tUnknown instruction: " << cpu->memory->get<byte>(address) << std::endl;
}

void Debugger::prompt(CPU *cpu)
{
    bool done = false;
    char buf[64];

    while (!done) {
        std::cout << "> " << std::flush;
        fgets(buf, 64, stdin);
        switch (buf[0]) {
        case 'q':
            exit(0);
        case 'r':
            std::cout << "\tA: " << cpu->a << "\tF: " << cpu->f << "\tAF: " << cpu->af << std::endl
                      << "\tB: " << cpu->b << "\tC: " << cpu->c << "\tBC: " << cpu->bc << std::endl
                      << "\tD: " << cpu->d << "\tE: " << cpu->e << "\tDE: " << cpu->de << std::endl
                      << "\tH: " << cpu->h << "\tL: " << cpu->l << "\tHL: " << cpu->hl << std::endl
                      << "\tPC: " << cpu->pc << "\tSP: " << cpu->sp << std::endl
                      << std::endl
                      << "\tIE: " << cpu->IE << "\tIF: " << cpu->IF << "\tIME: " << cpu->ime << std::endl;
            break;
        case 'i':
            printInstruction(cpu, cpu->pc);
            break;
        case 'b':
            if (strlen(buf) < 4) {
                listBreakpoints();
            } else {
                int v;
                sscanf(buf, "b %04x", &v);
                toggleBreakpoint(v);
            }
            break;
        case 'w':
            if (strlen(buf) < 4) {
                listWatches();
            } else {
                int v;
                sscanf(buf, "w %04x", &v);
                toggleWatch(v);
            }
            break;
        case 'p': {
            word w = cpu->pc;
            for (int i = 0; i < 16; i++) {
                printInstruction(cpu, w);
                Instruction *instruction = cpu->findInstruction(w);
                if (instruction) {
                    w += instruction->length;
                } else {
                    break;
                }
            }
            break;
        }
        case 'n':
            stepMode = true;
            done = true;
            break;
        case 'c':
            stepMode = false;
            done = true;
            break;
        case 'v':
            verboseCPU = !verboseCPU;
            std::cout << "verbose cpu = " << verboseCPU << std::endl;
            break;
        case 'u':
            verboseMemory = !verboseMemory;
            std::cout << "verbose memory = " << verboseMemory << std::endl;
            break;
        case 's':
            showStack(cpu);
            break;
        case 'm': {
            int v;
            sscanf(buf, "m %04x", &v);
            showMemory(cpu, v);
            break;
        }
        case 'h':
            puts("b - list/toggle breakpoint(s)");
            puts("c - continue");
            puts("e - list watches");
            puts("i - print current instruction");
            puts("n - next");
            puts("m - show memory at address");
            puts("p - print next 16 instructions");
            puts("q - quit");
            puts("r - print registers");
            puts("s - show stack");
            puts("u - toggle verbose memory");
            puts("v - toggle verbose cpu");
            puts("w - list/toggle watch(es)");
            break;
        }
    }
}
