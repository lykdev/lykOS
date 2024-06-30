#pragma once

#include <lib/def.h>

#define PIT_FREQ 1193182ull

/**
 * @brief Set the reload value.
 */
void x86_64_pit_set_reload_value(u16 reload_value);

/**
 * @brief Retrieve current PIT count.
 * @return Current PIT count.
 */
u16 x86_64_pit_read_count();

void x86_64_pit_timer_poll(u32 ticks);
