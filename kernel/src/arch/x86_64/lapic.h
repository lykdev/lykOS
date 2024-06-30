#pragma once

#include <lib/def.h>

/**
 * @brief Stop the local APIC timer
 */
void x86_64_lapic_timer_stop();

/**
 * @brief Polls the local APIC timer.
 * @warning This function blocks until polling is done.
 * @param ticks Ticks to poll.
 */
void x86_64_lapic_timer_poll(u32 ticks);

/**
 * @brief Perform a oneshot event using the APIC timer.
 * @param vector Interrupt vector.
 * @param us Time in microseconds.
 */
void x86_64_lapic_timer_oneshot(u8 vector, u64 us);

/**
 * @brief Initialize the local APIC software.
 */
void x86_64_lapic_timer_init();
