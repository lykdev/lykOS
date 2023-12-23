#pragma once

#include <lib/utils.h>

#define MSR_APIC_BASE 0x1B
#define MSR_GS_BASE   0xC0000101

u64 msr_read(u64 msr);

void msr_write(u64 msr, u64 value);
