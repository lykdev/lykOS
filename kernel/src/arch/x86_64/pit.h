#pragma once

#include <lib/def.h>

/*
    Inlining helps with the accuracy of the readings.
*/

void x86_64_pit_set_frequency(u64 frequency) __attribute__((always_inline));

u16 x86_64_pit_count() __attribute__((always_inline));
