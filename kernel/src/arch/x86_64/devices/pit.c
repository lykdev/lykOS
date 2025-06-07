#include "pit.h"

#include <arch/x86_64/io.h>

#define PIT_DATA_0 0x40
#define PIT_DATA_1 0x41
#define PIT_DATA_2 0x42
#define PIT_CMD    0x43

void x86_64_pit_set_reload(u16 reload_value)
{
    x86_64_io_outb(PIT_CMD, 0x34);
    x86_64_io_outb(PIT_DATA_0, (u8)reload_value);
    x86_64_io_outb(PIT_DATA_0, (u8)(reload_value >> 8));
}

void x86_64_pit_set_frequency(u64 frequency)
{
    u16 divisor = X86_64_PIT_BASE_FREQ / frequency;
    if(X86_64_PIT_BASE_FREQ % frequency > frequency / 2) // Round up.
        divisor++;
    x86_64_pit_set_reload(divisor);
}

u16 x86_64_pit_count()
{
    x86_64_io_outb(PIT_CMD, 0);
    u16 low  = x86_64_io_inb(PIT_DATA_0);
    u16 high = x86_64_io_inb(PIT_DATA_0);
    return (high << 8) | low;
}
