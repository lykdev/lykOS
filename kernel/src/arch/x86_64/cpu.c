#include <arch/cpu.h>

void arch_cpu_halt()
{
    while (true)
        __asm__ volatile ("hlt");
}

void arch_cpu_relax()
{
    __asm__ volatile ("pause");
}
