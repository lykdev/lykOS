#pragma once

#include <lib/def.h>

void arch_int_register_exception_handler(uint exception, void (*handler)());

uint arch_int_request_irq();

void arch_int_register_irq_handler(uint irq, void (*handler)());
