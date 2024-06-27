#include "panic.h"

#include <arch/cpu.h>

#include <lib/def.h>
#include <lib/log.h>

void panic(const char *format, ...)
{
    va_list list;
    va_start(list);

    _log(0xFF0000, format, list);

    va_end(list);

    cpu_halt();
}