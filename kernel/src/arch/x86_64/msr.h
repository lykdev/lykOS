#pragma once

#include <lib/utils.h>

#define MSR_APIC_BASE 0x1B
// If the processor is a bootstrap processor.
#define MSR_APIC_BASE_BSP 0x100 
#define MSR_APIC_BASE_ENABLE 0x800

u64 msr_read(u64 msr);

void msr_write(u64 msr, u64 value);
