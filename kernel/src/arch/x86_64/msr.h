#pragma once

#include <lib/def.h>

typedef enum
{
    X86_64_MSR_APIC_BASE       = 0x1B,
    X86_64_MSR_PAT             = 0x277,
    X86_64_MSR_EFER            = 0xC0000080,
    X86_64_MSR_STAR            = 0xC0000081,
    X86_64_MSR_LSTAR           = 0xC0000082,
    X86_64_MSR_CSTAR           = 0xC0000083,
    X86_64_MSR_SFMASK          = 0xC0000084,
    X86_64_MSR_FS_BASE         = 0xC0000100,
    X86_64_MSR_GS_BASE         = 0xC0000101,
    X86_64_MSR_KERNEL_GS_BASE  = 0xC0000102
}
x86_64_msr_t;

u64 x86_64_msr_read(u64 msr);

void x86_64_msr_write(x86_64_msr_t msr, u64 value);
