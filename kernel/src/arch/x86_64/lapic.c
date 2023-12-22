#include "lapic.h"

#include <arch/x86_64/msr.h>

// Check the Intel SDM: System Programming Guide 11.4.5
#define APIC_BASE_MASK ~(0xFFFull)


#define REG_SPURIOUS 0xF0

// The timer's config register.
#define REG_LVT_TIMER 0x320
// The timer's initial count.
#define REG_TIMER_INIT 0x380
// The current count of the timer.
#define REG_TIMER_CURR 0x390
// Specifies by how much to divide the decrement speed.
// Default (and maximum) decrement speed = CPU bus frequency.
#define REG_TIMER_DIV  0x3E0

// The MSR_APIC_BASE points to the memory mapped configuration space for the current processor's local APIC.
// It's absolutely normal for the MSR_APIC_BASE to be the same for each core as it doesn't point to actual RAM.
// This region of the address space is hard linked to the current processor.

static void lapic_write(u32 reg, u32 data)
{
    *(u32*)((msr_read(MSR_APIC_BASE) & APIC_BASE_MASK) + reg + HHDM) = data;
}

static u32 lapic_read(u32 reg)
{
    return *(u32*)((msr_read(MSR_APIC_BASE) & APIC_BASE_MASK) + reg + HHDM);
}

// 11.5.1
static void lapic_stop()
{
    lapic_write(REG_LVT_TIMER, 1 << 16); // Mask interrupts
    lapic_write(REG_TIMER_INIT, 0);
}


void lapic_init()
{
    lapic_write(REG_SPURIOUS, 0x1FF);
    lapic_stop();
}

// 11.5.4
void lapic_timer_oneshoot(u8 vec, u64 ticks)
{
    lapic_stop();

    lapic_write(REG_LVT_TIMER, vec); // Tell the timer which interrupt to trigger.
    lapic_write(REG_TIMER_DIV, 0); // Divide by 2 (lowest value possible).
    lapic_write(REG_TIMER_INIT, ticks); // Tell the timer where to start from.
}