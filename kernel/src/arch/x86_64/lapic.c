#include "lapic.h"

#include <arch/x86_64/io.h>
#include <arch/x86_64/msr.h>
#include <arch/x86_64/pit.h>

#include <lib/def.h>
#include <lib/hhdm.h>
#include <lib/log.h>

#define BASE_MASK 0xFFFFFFFFFF000

#define LVT_TIMER_REG       0x320
#define SPURIOUS_REG        0x0F0

#define TIMER_DIVISOR       0x3E0
#define TIMER_INITIAL_COUNT 0x380
#define TIMER_CURRENT_COUNT 0x390

static u64 lapic_timer_frequency;

static inline void lapic_write(u32 reg, u32 data)
{
    *(volatile u32*)((x86_64_msr_read(X86_64_MSR_APIC_BASE) & BASE_MASK) + reg + HHDM) = data;
}

static inline u32 lapic_read(u32 reg)
{
    return *(volatile u32*)((x86_64_msr_read(X86_64_MSR_APIC_BASE) & BASE_MASK) + reg + HHDM);
}

void x86_64_lapic_timer_stop()
{
    lapic_write(TIMER_INITIAL_COUNT, 0);   // Stop the timer.
    lapic_write(LVT_TIMER_REG, (1 << 16)); // Mask the timer interrupt.   
}

void x86_64_lapic_timer_poll(u32 ticks)
{
    x86_64_lapic_timer_stop();

    lapic_write(TIMER_DIVISOR, 0);
    lapic_write(TIMER_INITIAL_COUNT, ticks);

    while(lapic_read(TIMER_CURRENT_COUNT) != 0)
        ;
        
    x86_64_lapic_timer_stop();
}

void x86_64_lapic_timer_oneshot(u8 vector, u64 us)
{
    x86_64_lapic_timer_stop();

    lapic_write(LVT_TIMER_REG, vector); // Interrupt vector to be triggered.
    lapic_write(TIMER_DIVISOR, 0);      // Divisor rate set to 2.
    lapic_write(TIMER_INITIAL_COUNT, us * (lapic_timer_frequency / 1'000'000));
}

void x86_64_lapic_timer_init()
{
    // Enable APIC software.
    lapic_write(SPURIOUS_REG, 1 << 8); 

    // Calculate lapic timer frequency.
    x86_64_pit_set_reload_value(UINT16_MAX);
    u64 start_count = x86_64_pit_read_count();
    x86_64_lapic_timer_poll(0x10000);
    u64 end_count = x86_64_pit_read_count();

    lapic_timer_frequency = (u64)((u64)0x10000 / (start_count - end_count)) * PIT_FREQ;

    log("LAPIC timer freq: %u", lapic_timer_frequency);
}   
