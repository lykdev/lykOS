#include "log.h"

#include <arch/serial.h>

#include <lib/def.h>
#include <lib/printf.h>
#include <lib/slock.h>

static slock_t slock = SLOCK_INIT;

void _log(u32 color, const char *format, va_list list)
{
    slock_acquire(&slock);

    char buf[256];
    vsnprintf(buf, 256, format, list);

    serial_send_str(buf);
    serial_send_char('\n');

    slock_release(&slock);
}

void log(const char *format, ...)
{
    va_list list;
    va_start(list);

    _log(0xFFFFFF, format, list);

    va_end(list);
}

void log_warn(const char *format, ...)
{
    va_list list;
    va_start(list);

    _log(0xFFFF00, format, list);

    va_end(list);
}

void log_err(const char *format, ...)
{
    va_list list;
    va_start(list);

    _log(0xFF0000, format, list);

    va_end(list);
}
