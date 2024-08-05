#pragma once

#include <core/tasking/task.h>

/// @brief Halts the current CPU core.
void cpu_lcore_halt();

/// @brief Improves spinlock performance by hinting the current CPU core that the code sequence is a spin-wait loop.
void cpu_lcore_relax();

task_t *cpu_lcore_get_current_task();

void cpu_lcore_set_current_task(task_t *t);
