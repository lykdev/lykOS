#pragma once

#include <lib/def.h>

typedef struct
{
#if defined(__x86_64__)
    void *tss;
#elif defined(__aarch64__)
    #error Undefined.
#endif
}
arch_cpu_context_t;

void arch_cpu_context_init(arch_cpu_context_t *context);

void *arch_cpu_read_thread_reg();

void arch_cpu_write_thread_reg(void *t);

[[noreturn]] void arch_cpu_halt();

void arch_cpu_relax();

void arch_cpu_int_unmask();

void arch_cpu_int_mask();

bool arch_cpu_int_enabled();
