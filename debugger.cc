#include <algorithm>

#include <stdio.h>
#include <stdlib.h>

#include "word.h"
#include "cpu.h"
#include "debugger.h"

Debugger::Debugger() : verboseCPU(false), stepMode(true)
{
}

void Debugger::toggleBreakpoint(word address)
{
    Breakpoints::iterator it = std::find(breakpoints.begin(), breakpoints.end(), address);
    if(it == breakpoints.end()) {
        printf("Set breakpoint %04x\n", address.value());
        breakpoints.push_back(address);
    } else {
        printf("Remove breakpoint %04x\n", address.value());
        breakpoints.erase(it);
    }
}

void Debugger::listBreakpoints()
{
    if (breakpoints.empty()) {
        printf("No breakpoints set\n");
    } else {
        printf("List breakpoints:\n");
        for (Breakpoints::iterator it = breakpoints.begin(); it != breakpoints.end(); ++it) {
            printf("\t%04x\n", (*it).value());
        }
    }
}

void Debugger::handleInstruction(CPU *cpu, word address)
{
    if (stepMode) {
        printInstruction(cpu, address);
        prompt(cpu);
    } else if(std::find(breakpoints.begin(), breakpoints.end(), address) != breakpoints.end()) {
        printf("Breakpoint at\n");
        printInstruction(cpu, address);
        prompt(cpu);
    } else if (verboseCPU) {
        printInstruction(cpu, address);
    }
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
            printf("\n\t%04x", i.value());
        }

        if (i == address) {
            printf(" \x1b[31m%02x\x1b[0m", cpu->memory->get<byte>(i));
        } else {
            printf(" %02x", cpu->memory->get<byte>(i));
        }
        if (i == end)
            break;
        i++;
    }
    printf("\n");
}

void Debugger::showStack(CPU *cpu)
{
    word start = cpu->sp + word(8);
    if (start < 0x0010)
        start = 0xffff;
    for (word i = start; i >= cpu->sp; i--) {
        if (i == cpu->sp)
            printf("\t\x1b[31m%04x %02x\x1b[0m\n", i.value(), cpu->memory->get<byte>(i));
        else
            printf("\t%04x %02x\n", i.value(), cpu->memory->get<byte>(i));
    }
}

void Debugger::printInstruction(CPU *cpu, word address)
{
    word i = 0;
    Command * cmd = cpu->findCommand(address);
    if (cmd) {
        printf("\t%04x\t", address.value());
        for (i = 0; i < 3; i++) {
            if (i < cmd->length) {
                printf("%02x ", cpu->memory->get<byte>(address+i));
            } else {
                printf("   ");
            }
        }
        printf("\t%s\n", cmd->mnemonic);
        return;
    }

    printf("\t%04x\tUnknown instruction: %02x\n", address.value(), cpu->memory->get<byte>(address));
}

void Debugger::prompt(CPU *cpu)
{
    bool done = false;
    char buf[64];

    while (!done) {
        printf("> ");
        fflush(stdout);
        fgets(buf, 64, stdin);
        switch (buf[0]) {
        case 'q':
            exit(0);
        case 'r':
            printf("\tA: %02x\tF: %02x\tAF: %04x\n", cpu->a, cpu->f, cpu->af.value());
            printf("\tB: %02x\tC: %02x\tBC: %04x\n", cpu->b, cpu->c, cpu->bc.value());
            printf("\tD: %02x\tE: %02x\tDE: %04x\n", cpu->d, cpu->e, cpu->de.value());
            printf("\tH: %02x\tL: %02x\tHL: %04x\n", cpu->h, cpu->l, cpu->hl.value());
            printf("\tPC: %04x\tSP: %04x\n", cpu->pc.value(), cpu->sp.value());
            break;
        case 'i':
            printInstruction(cpu, cpu->pc);
            break;
        case 'b': {
            word w;
            sscanf(buf, "b %04x", &w.d.w);
            toggleBreakpoint(w);
            break;
        }
        case 'l':
            listBreakpoints();
            break;
        case 'p': {
            word w = cpu->pc;
            for (int i = 0; i < 16; i++) {
                printInstruction(cpu, w);
                Command *cmd = cpu->findCommand(w);
                if (cmd) {
                    w += cmd->length;
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
            printf("verbose cpu = %d\n", verboseCPU);
            break;
        case 's':
            showStack(cpu);
            break;
        case 'm': {
            word w;
            sscanf(buf, "m %04x", &w.d.w);
            showMemory(cpu, w);
            break;
        }
        case 'h':
            puts("b - toggle breakpoint");
            puts("c - continue");
            puts("i - print current instruction");
            puts("l - list breakpoints");
            puts("n - next");
            puts("m - show memory at address");
            puts("p - print next 16 instructions");
            puts("q - quit");
            puts("r - print registers");
            puts("s - show stack");
            puts("v - toggle verbose cpu");
            break;
        }
    }
}
