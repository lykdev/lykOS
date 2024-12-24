#pragma once

#include <utils/def.h>

void* arch_cpu_read_thread_reg();

void arch_cpu_write_thread_reg(void *t);

void arch_cpu_halt();

void arch_cpu_relax();
