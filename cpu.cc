#include <iostream>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "debugger.h"
#include "word.h"
#include "memory.h"
#include "instructions.h"
#include "instructionset.h"
#include "references.h"
#include "base_instructionset.h"

CPU::CPU(Memory *memory, Debugger *debugger)
    : memory(memory),
      debugger(debugger),
      ime(1),
      cycles(0),
      pc(registerBank[0]), pc_hi(registerBank[0].hiRef()), pc_lo(registerBank[0].loRef()),
      sp(registerBank[1]),
      af(registerBank[2]), a(registerBank[2].hiRef()), f(registerBank[2].loRef()),
      bc(registerBank[3]), b(registerBank[3].hiRef()), c(registerBank[3].loRef()),
      de(registerBank[4]), d(registerBank[4].hiRef()), e(registerBank[4].loRef()),
      hl(registerBank[5]), h(registerBank[5].hiRef()), l(registerBank[5].loRef()),
      ly(memory->getRef(0xff44)), IE(memory->getRef(0xffff)), IF(memory->getRef(0xff0f))
{
    pc = 0x100;
    sp = 0xFFFE;
    af = 0x01;

    b  = 0x00;
    c  = 0x13;
    de = 0x00D8;
    hl = 0x014D;

    ly = 0x00;

    instructionSet = new InstructionSet();
    initialize_base_instructionset(instructionSet, this);
}

CPU::~CPU()
{
    delete instructionSet;
}

Instruction * CPU::findInstruction(word address)
{
    return instructionSet->findInstruction(memory->get<byte>(address));
}

void CPU::step()
{
    // Check for interrupts...
    byte irqs = IE & IF;
    if (ime && irqs) {
        if (irqs & (INT_VBLANK+1))
            callInterrupt(INT_VBLANK,  0x0040);
        else if (irqs & (INT_LCDSTAT+1))
            callInterrupt(INT_LCDSTAT, 0x0048);
        else if (irqs & (INT_TIMER+1))
            callInterrupt(INT_TIMER,   0x0050);
        //else if (irqs & (INT_SERIAL+1))
        //    callInterrupt(INT_SERIAL,  0x0058);
        else if (irqs & (INT_JOYPAD+1))
            callInterrupt(INT_JOYPAD,  0x0060);
    }

    // Process command...
    Instruction * cmd = findInstruction(pc);
    if (cmd) {
        debugger->handleInstruction(this, pc);
        pc++;
        if (cmd->condition) {
            if ((*cmd->condition)(this)) {
                cmd->run();
                cycles += cmd->cycles0;
            } else {
                cycles += cmd->cycles1;
                pc += cmd->length-1;
            }
        } else {
            cmd->run();
            cycles += cmd->cycles0;
        }
    } else {
        std::cerr << pc << " *** Unknown machine code: " << memory->get<byte>(pc) << std::endl;
        debugger->prompt(this);
    }
}

void CPU::callInterrupt(Interrupt irq, word address)
{
    // Reset interrupt flag in IF
    IF &= ~(1 << irq);

    // Reset IME
    ime = 0;

    // Push current PC on stack...
    sp--;
    memory->set(sp, pc_hi);
    sp--;
    memory->set(sp, pc_lo);

    // Set new PC to interrupt address
    pc = address;

    debugger->handleInterrupt(irq, address);
}

void CPU::requestInterrupt(Interrupt irq)
{
    IF |= (1 << irq);
}
