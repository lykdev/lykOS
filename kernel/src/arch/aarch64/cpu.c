#include <arch/cpu.h>

void *arch_cpu_read_thread_reg()
{
    void *thread = NULL;
    asm volatile("mrs %0, tpidr_el1" : "=r"(thread));
    return thread;
}

void arch_cpu_write_thread_reg(void *t)
{
    asm volatile("msr tpidr_el1, %0" : : "r"(t));
}

void arch_cpu_halt()
{
    while (true)
        asm volatile("wfi");
}

void arch_cpu_relax()
{
    asm volatile("yield");
}

void arch_cpu_int_unmask()
{
    asm volatile("msr daifclr, #0b1111");
}

void arch_cpu_int_mask()
{
    asm volatile("msr daifset, #0b1111");
}
