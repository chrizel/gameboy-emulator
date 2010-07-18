#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "debugger.h"

void gbDebugPrintInstruction(CPU *cpu, word address)
{
    int i = 0;
    Command * cmd = cpu->findCommand(address);
    if (cmd) {
        printf("\t%04x\t", address);
        for (i = 0; i < 3; i++) {
            if (i < cmd->length) {
                printf("%02x ", cpu->memory->get(address+i));
            } else {
                printf("   ");
            }
        }
        printf("\t%s\n", cmd->mnemonic);
        return;
    }

    printf("\t%04x\tUnknown instruction: %02x\n", address, cpu->memory->get(address));
}

void gbDebugPrompt(CPU *cpu)
{
    int done = 0;
    char buf[64];

    while (!done) {
        printf("> ");
        fflush(stdout);
        fgets(buf, 64, stdin);
        switch (buf[0]) {
        case 'q':
            exit(0);
        case 'r':
            printf("\tA: %02x\tF: %02x\tAF: %04x\n", cpu->a, cpu->f, cpu->af);
            printf("\tB: %02x\tC: %02x\tBC: %04x\n", cpu->b, cpu->c, cpu->bc);
            printf("\tD: %02x\tE: %02x\tDE: %04x\n", cpu->d, cpu->e, cpu->de);
            printf("\tH: %02x\tL: %02x\tHL: %04x\n", cpu->h, cpu->l, cpu->hl);
            printf("\tPC: %04x\tSP: %04x\n", cpu->pc, cpu->sp);
            break;
        case 'i':
            gbDebugPrintInstruction(cpu, cpu->pc);
            break;
        case 'd':
            word w = cpu->pc;
            for (int i = 0; i < 16; i++) {
                gbDebugPrintInstruction(cpu, w);
                Command *cmd = cpu->findCommand(w);
                if (cmd) {
                    w += cmd->length;
                } else {
                    break;
                }
            }
            break;
        case 'n':
            done = 1;
            break;
        }
    }
}
