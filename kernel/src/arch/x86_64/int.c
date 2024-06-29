#include <arch/int.h>

void int_enable()
{
    __asm__ volatile ("sti");
}

void int_disable()
{
    __asm__ volatile ("cli");
}
