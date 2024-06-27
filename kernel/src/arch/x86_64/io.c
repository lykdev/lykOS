#include "io.h"

void io_outb(u16 port, u8 val)
{
    __asm__ volatile("outb %0, %1"
                 :
                 : "a"(val), "Nd"(port));
}

u8 io_inb(u16 port)
{
    u8 ret;
    __asm__ volatile("inb %1, %0"
                 : "=a"(ret)
                 : "Nd"(port));
    return ret;
}

void io_outl(u16 port, u32 val)
{
    __asm__ volatile("outl %0, %w1" : : "a" (val), "Nd" (port));
}

u32 io_inl(u16 port)
{
    u32 data;
    __asm__ volatile("inl %w1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

void io_outw(u16 port, u16 val)
{
    __asm__ volatile("outw %w0, %w1" : : "a" (val), "Nd" (port));
}


u16 io_inw(u16 port)
{
    u16 data;
    __asm__ volatile("inw %w1, %w0" : "=a" (data) : "Nd" (port));
    return data;
}
