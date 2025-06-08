#pragma once

#include <lib/def.h>

void arch_timer_stop();

void arch_timer_oneshoot(uint irq, u64 us);
