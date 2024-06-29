#include "io.h"

void x86_64_io_outb(u16 port, u8 val)
{
    __asm__ volatile("outb %0, %1"
                    :
                    : "a"(val), "Nd"(port));
    }

u8 x86_64_io_inb(u16 port)
{
    u8 ret;
    __asm__ volatile("inb %1, %0"
                    : "=a"(ret)
                    : "Nd"(port));
    return ret;
}

void x86_64_io_outl(u16 port, u32 val)
{
    __asm__ volatile("outl %0, %1"
                    :
                    : "a" (val), "Nd" (port));
}

u32 x86_64_io_inl(u16 port)
{
    u32 ret;
    __asm__ volatile("inl %1, %0"
                    : "=a" (ret)
                    : "Nd" (port));
    return ret;
}
