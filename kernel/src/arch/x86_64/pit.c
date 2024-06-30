#include "pit.h"

#include <arch/x86_64/io.h>

#define CHANNEL0_DATA 0x40
#define CHANNEL1_DATA 0x41
#define CHANNEL2_DATA 0x42
#define CMD 0x43

void x86_64_pit_set_reload_value(u16 reload_value)
{
    x86_64_io_outb(CMD, 0b00110100); // Channel 0, Access mode: lobyte/hibyte, rate generator.

    x86_64_io_outb(CHANNEL0_DATA, (u8)reload_value);
    x86_64_io_outb(CHANNEL0_DATA, (u8)(reload_value >> 8));
}

u16 x86_64_pit_read_count()
{
    x86_64_io_outb(CMD, 0); // Channel 0.

    u16 low = x86_64_io_inb(CHANNEL0_DATA);
    u16 high = x86_64_io_inb(CHANNEL0_DATA);

    return (high << 8) | low;
}

void x86_64_pit_timer_poll(u32 ticks)
{
    x86_64_pit_set_reload_value(ticks);

    while(x86_64_pit_read_count() > 100)
        ;
}