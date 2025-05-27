#pragma once

#include <lib/def.h>

void x86_64_lapic_send_eoi();

void x86_64_lapic_ipi(u32 lapic_id, u32 vec);

void x86_64_lapic_init();
