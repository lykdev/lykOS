#include "arch/cpu.h"

#include <arch/x86_64/msr.h>

void cpu_halt()
{
    while (true)
        __asm__ volatile ("hlt");
}

void cpu_relax()
{
    __asm__ volatile ("pause");
}

task_t *cpu_get_current_task()
{
    task_t *ret;
    asm volatile("mov %%gs:0, %0" : "=r" (ret));
    
    return ret;
}

void *cpu_set_current_task(task_t *t)
{
    x86_64_msr_write(X86_64_MSR_GS_BASE, (u64)t);
}
