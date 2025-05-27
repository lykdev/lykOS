#pragma once

#include <lib/def.h>

typedef struct
{

}
arch_cpu_info_t;

void *arch_cpu_read_thread_reg();

void arch_cpu_write_thread_reg(void *t);

void arch_cpu_halt();

void arch_cpu_relax();

void arch_cpu_int_unmask();

void arch_cpu_int_mask();

bool arch_cpu_int_enabled();
