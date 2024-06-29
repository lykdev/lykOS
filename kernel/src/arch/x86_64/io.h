#pragma once

#include <lib/def.h>

void x86_64_io_outb(u16 port, u8 val);

void x86_64_io_outl(u16 port, u32 val);

u8 x86_64_io_inb(u16 port);

u32 x86_64_io_inl(u16 port);
