#pragma once

#include <lib/def.h>

void x86_64_ioapic_init();

void x86_64_ioapic_map_gsi(u8 gsi, u8 lapic_id, bool low_polarity, bool trigger_mode, u8 vector);

void x86_64_ioapic_map_legacy_irq(u8 irq, u8 lapic_id, bool fallback_low_polarity, bool fallback_trigger_mode, u8 vector);