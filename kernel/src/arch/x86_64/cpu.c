#include <arch/cpu.h>

#include <utils/log.h>

arch_cpu_core_t arch_cpu_cores[128];
size_t arch_cpu_core_count;

void arch_cpu_halt()
{
    while (true)
        __asm__ volatile ("hlt");
}

void arch_cpu_relax()
{
    __asm__ volatile ("pause");
}

void arch_cpu_core_init()
{
    log("NEW CORE");

    arch_cpu_halt();
}
