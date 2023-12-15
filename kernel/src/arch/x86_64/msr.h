#pragma once

#include <lib/utils.h>

u64 msr_read(u64 msr);

void msr_write(u64 msr, u64 value);
