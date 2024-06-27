#pragma once

#include <lib/def.h>

void io_outb(u16 port, u8 val);

void io_outl(u16 port, u32 val);

void io_outw(u16 port, u16 val);

u8 io_inb(u16 port);

u32 io_inl(u16 port);

u16 io_inw(u16 port);