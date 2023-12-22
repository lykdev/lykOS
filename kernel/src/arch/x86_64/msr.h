#pragma once

#include <lib/utils.h>

#define MSR_APIC_BASE 0x1B

u64 msr_read(u64 msr);

void msr_write(u64 msr, u64 value);
