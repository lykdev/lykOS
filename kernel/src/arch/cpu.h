#pragma once

/// @brief Halts the current CPU core.
void cpu_halt();

/// @brief Improves spinlock performance by hinting the current CPU core that the code sequence is a spin-wait loop.
void cpu_relax();