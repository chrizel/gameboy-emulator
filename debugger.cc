#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "debugger.h"

void gbDebugPrintInstruction(GameBoy *gb, word address)
{
    int i = 0;
    Command * cmd = gbCPUFindCommand(gb, address);
    if (cmd) {
        printf("\t%04x\t", address);
        for (i = 0; i < 3; i++) {
            if (i < cmd->length) {
                printf("%02x ", MEM(gb, address+i));
            } else {
                printf("   ");
            }
        }
        printf("\t%s\n", cmd->mnemonic);
        return;
    }

    printf("\t%04x\tUnknown instruction: %02x\n", address, MEM(gb, address));
}

void gbDebugPrompt(GameBoy *gb)
{
    int i = 0, done = 0;
    word w = 0;
    char buf[64];
    Command *cmd;

    while (!done) {
        printf("> ");
        fflush(stdout);
        fgets(buf, 64, stdin);
        switch (buf[0]) {
        case 'q':
            exit(0);
        case 'r':
            printf("\tA: %02x\tF: %02x\tAF: %04x\n", REG_A(gb), REG_F(gb), REG_AF(gb));
            printf("\tB: %02x\tC: %02x\tBC: %04x\n", REG_B(gb), REG_C(gb), REG_BC(gb));
            printf("\tD: %02x\tE: %02x\tDE: %04x\n", REG_D(gb), REG_E(gb), REG_DE(gb));
            printf("\tH: %02x\tL: %02x\tHL: %04x\n", REG_H(gb), REG_L(gb), REG_HL(gb));
            printf("\tPC: %04x\tSP: %04x\n", REG_PC(gb), REG_SP(gb));
            break;
        case 'i':
            gbDebugPrintInstruction(gb, REG_PC(gb));
            break;
        case 'd':
            w = REG_PC(gb);
            for (i = 0; i < 16; i++) {
                gbDebugPrintInstruction(gb, w);
                cmd = gbCPUFindCommand(gb, w);
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
