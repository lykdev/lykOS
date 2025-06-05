#include "msr.h"

u64 x86_64_msr_read(u64 msr)
{
    u32 low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((u64)high << 32) | low;
}

void x86_64_msr_write(x86_64_msr_t msr, u64 value)
{
    asm volatile("wrmsr" : : "a"((u32)value), "d"((u32)(value >> 32)), "c"(msr));
}
