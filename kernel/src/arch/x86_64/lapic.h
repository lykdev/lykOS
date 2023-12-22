#pragma once

#include <lib/utils.h>

void lapic_init();

void lapic_timer_oneshoot(u8 vec, u64 ticks);
