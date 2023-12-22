#pragma once

#include <lib/utils.h>

void cpu_core_setup();
void cpu_core_entry(struct limine_smp_info *lcore);
