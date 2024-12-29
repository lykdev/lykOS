#include <arch/cpu.h>

void* arch_cpu_read_thread_reg()
{
    void *thread = NULL;
    asm volatile("mrs %0, tpidr_el1" : "=r" (thread));
    return thread;
}

// Write the thread pointer to TPIDR_EL1
void arch_cpu_write_thread_reg(void *t)
{
    asm volatile("msr tpidr_el1, %0" : : "r" (t));
}

void arch_cpu_halt()
{
    while (true)
        __asm__ volatile("wfi");
}

void arch_cpu_relax()
{
    __asm__ volatile("yield");
}
