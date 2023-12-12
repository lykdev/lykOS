#include "exc.h"

#include <lib/utils.h>

extern void load_vec_table();
extern void exca();

void exc_enable()
{
    load_vec_table();

    log("Enabled exception handlers.");

    // Just a small test
    // exca();
}

void exc_handler(u64 type, u64 esr, u64 elr, u64 spsr, u64 far)
{
    switch(esr>>26)
    {
        case 0b000000: log("Unknown"); break;
        case 0b000001: log("Trapped WFI/WFE"); break;
        case 0b001110: log("Illegal execution"); break;
        case 0b010101: log("System call"); break;
        case 0b100000: log("Instruction abort, lower EL"); break;
        case 0b100001: log("Instruction abort, same EL"); break;
        case 0b100010: log("Instruction alignment fault"); break;
        case 0b100100: log("Data abort, lower EL"); break;
        case 0b100101: log("Data abort, same EL"); break;
        case 0b100110: log("Stack alignment fault"); break;
        case 0b101100: log("Floating point"); break;
        case 0b111100: log("Breakpoint"); break;
        default: log("Unknown. Exception Class: %b", (esr>>26) & 0b111111); break;
    }

    while(1);
}