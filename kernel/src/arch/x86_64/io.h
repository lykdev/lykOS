#pragma once

#include <utils/def.h>

static inline void x86_64_io_outb(u16 port, u8 val)
{
    __asm__ volatile("outb %0, %1"
                 :
                 : "a"(val), "Nd"(port));
}

static inline u8 x86_64_io_inb(u16 port)
{
    u8 ret;
    __asm__ volatile("inb %1, %0"
                 : "=a"(ret)
                 : "Nd"(port));
    return ret;
}

static inline void x86_64_io_outl(u16 port, u32 val)
{
    __asm__ volatile("outl %0, %w1" : : "a" (val), "Nd" (port));
}

static inline u32 x86_64_io_inl(u16 port)
{
    u32 data;
    __asm__ volatile("inl %w1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline void x86_64_io_outw(u16 port, u16 val)
{
    __asm__ volatile("outw %w0, %w1" : : "a" (val), "Nd" (port));
}

static inline u16 x86_64_io_inw(u16 port)
{
    u16 data;
    __asm__ volatile("inw %w1, %w0" : "=a" (data) : "Nd" (port));
    return data;
}
