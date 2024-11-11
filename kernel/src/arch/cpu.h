#pragma once

#include <utils/def.h>

typedef struct
{
    u8 id;
}
arch_cpu_core_t;
extern arch_cpu_core_t arch_cpu_cores[128];
extern size_t arch_cpu_core_count;

void arch_cpu_halt();

void arch_cpu_relax();

void arch_cpu_core_init();
