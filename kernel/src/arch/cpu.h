#pragma once

#include <lib/def.h>

void *arch_cpu_read_thread_reg() __attribute__((always_inline));

void arch_cpu_write_thread_reg(void *t) __attribute__((always_inline));

void arch_cpu_halt() __attribute__((always_inline));

void arch_cpu_relax() __attribute__((always_inline));

void arch_cpu_int_unmask() __attribute__((always_inline));

void arch_cpu_int_mask() __attribute__((always_inline));

bool arch_cpu_int_enabled() __attribute__((always_inline));
