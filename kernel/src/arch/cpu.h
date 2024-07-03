#pragma once

#include <core/sched/task.h>

/// @brief Halts the current CPU core.
void cpu_halt();

/// @brief Improves spinlock performance by hinting the current CPU core that the code sequence is a spin-wait loop.
void cpu_relax();

task_t *cpu_get_current_task();

void *cpu_set_current_task(task_t *t);
