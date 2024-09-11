#include <arch/cpu.h>

void arch_cpu_halt()
{
    while (true)
        __asm__ volatile("wfi");
}

void arch_cpu_relax()
{
    __asm__ volatile("yield");
}
