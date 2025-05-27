#include "lapic.h"

#include <arch/timer.h>
#include <arch/x86_64/msr.h>
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

static inline void lapic_write(u32 reg, u32 data)
{
    *(volatile u32*)(g_lapic_base + reg) = data;
}

static inline u32 lapic_read(uint32_t reg)
{
    return *(volatile u32*)(g_lapic_base + reg);
}

void arch_timer_stop()
{
    lapic_write(REG_TIMER_LVT, MASK);
    lapic_write(REG_TIMER_INITIAL_COUNT, 0);
}

void arch_timer_oneshoot(u8 vector, u8 ms)
{
    arch_timer_stop();
    lapic_write(REG_TIMER_LVT, ONE_SHOOT | vector);
    lapic_write(REG_TIMER_DIV, 0);
    // lapic_write(REG_TIMER_INITIAL_COUNT, ms * (X86_64_CPU_CURRENT.lapic_timer_frequency / 1'000'000));
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

    log("LAPIC initialized.");
}
