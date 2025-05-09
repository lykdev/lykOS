#pragma once

#include <lib/def.h>

void arch_int_irq_register_handler(uint irq, void (*handler)());
