#include "panic.h"

#include <arch/cpu.h>

#include <utils/def.h>
#include <utils/log.h>

void panic(const char *format, ...)
{
    va_list list;
    va_start(list);

    _n_log("", format, list);

    va_end(list);

    arch_cpu_halt();
}
