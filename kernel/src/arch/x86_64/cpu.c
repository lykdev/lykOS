#include "arch/cpu.h"

void cpu_halt()
{
    while (true)
        __asm__ volatile ("hlt");
}

void cpu_relax()
{
    __asm__ volatile ("pause");
}