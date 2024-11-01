#include <arch/int.h>

void arch_int_unmask()
{
    asm volatile ("msr daifclr, #0b1111");
}

void arch_int_mask()
{
    asm volatile ("msr daifset, #0b1111");
}
