#include "panic.h"

#include <arch/cpu.h>

#include <common/log.h>
#include <lib/def.h>

void panic(const char *format, ...)
{
    log("---PANIC---");

    va_list list;
    va_start(list);

    _n_log(format, list);

    va_end(list);

    arch_cpu_halt();
}
