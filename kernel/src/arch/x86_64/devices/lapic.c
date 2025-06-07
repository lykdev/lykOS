#include "lapic.h"

#include <arch/timer.h>
#include <arch/x86_64/msr.h>
#include <arch/x86_64/devices/pit.h>
#include <common/hhdm.h>
#include <common/log.h>

#define REG_ID 0x20
#define REG_SPURIOUS 0xF0
#define REG_EOI 0xB0
#define REG_IN_SERVICE_BASE 0x100
#define REG_ICR0 0x300
#define REG_ICR1 0x310
#define REG_TIMER_LVT 0x320
#define REG_TIMER_DIV 0x3E0
#define REG_TIMER_INITIAL_COUNT 0x380
#define REG_TIMER_CURRENT_COUNT 0x390

#define ONE_SHOOT    (0 << 17)
#define PERIODIC     (1 << 17)
#define TSC_DEADLINE (2 << 17)
#define MASK         (1 << 16)

static u64 g_lapic_base;
static u64 g_lapic_timer_freq = 0;

static inline void lapic_write(u32 reg, u32 data)
{
    *(volatile u32*)(g_lapic_base + reg) = data;
}

static inline u32 lapic_read(uint32_t reg)
{
    return *(volatile u32*)(g_lapic_base + reg);
}

void x86_64_lapic_timer_stop()
{
    lapic_write(REG_TIMER_LVT, MASK);
    lapic_write(REG_TIMER_INITIAL_COUNT, 0);
}

void x86_64_lapic_timer_oneshoot(u8 vector, u8 us)
{
    x86_64_lapic_timer_stop();
    lapic_write(REG_TIMER_LVT, ONE_SHOOT | vector);
    lapic_write(REG_TIMER_DIV, 0);
    lapic_write(REG_TIMER_INITIAL_COUNT, us * (g_lapic_timer_freq / 1'000'000));
}

void x86_64_lapic_send_eoi()
{
    lapic_write(REG_EOI, 0);
}

void x86_64_lapic_ipi(u32 lapic_id, u32 vec)
{
    lapic_write(REG_ICR1, lapic_id << 24);
    lapic_write(REG_ICR0, vec);
}

void x86_64_lapic_init()
{
    g_lapic_base = (x86_64_msr_read(X86_64_MSR_APIC_BASE) & 0xFFFFFFFFFF000) + HHDM;
    lapic_write(REG_SPURIOUS, (1 << 8) | 0xFF);

    if (!g_lapic_timer_freq)
    {
        lapic_write(REG_TIMER_DIV, 0);
        for (u64 lapic_ticks = 8; /* :) */; lapic_ticks *= 2)
        {
            x86_64_pit_set_reload(UINT16_MAX);
            u16 pit_start = x86_64_pit_count();

            // Start LAPIC timer with `lapic_ticks` and wait for it to count down to 0.
            lapic_write(REG_TIMER_LVT, MASK);
            lapic_write(REG_TIMER_INITIAL_COUNT, lapic_ticks);
            while(lapic_read(REG_TIMER_CURRENT_COUNT) != 0)
                ;
            lapic_write(REG_TIMER_LVT, MASK);

            u16 pit_end = x86_64_pit_count();
            // Compute how many PIT ticks elapsed during the LAPIC countdown.
            u16 pit_delta = pit_start - pit_end;

            if (pit_delta < UINT16_MAX / 4)
                continue;

            // Compute LAPIC timer frequency. Multiplications go first to avoid truncation.
            // (lapic_ticks / pit_delta) * X86_64_PIT_BASE_FREQ
            g_lapic_timer_freq = lapic_ticks * X86_64_PIT_BASE_FREQ / pit_delta;
            log("LAPIC timer calibrated. Timer freq: %uHz", g_lapic_timer_freq);
            break;
        }
    }

    log("LAPIC initialized.");
}
