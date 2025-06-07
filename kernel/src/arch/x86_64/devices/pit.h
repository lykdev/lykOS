#pragma once

#include <lib/def.h>

#define X86_64_PIT_BASE_FREQ 1'193'180

void x86_64_pit_set_reload(u16 reload_value);

void x86_64_pit_set_frequency(u64 frequency);

u16 x86_64_pit_count();
