#include "log.h"

#include <arch/serial.h>
#include <common/sync/spinlock.h>
#include <graphics/draw.h>
#include <graphics/font_basic.h>
#include <lib/printf.h>
#include <lib/string.h>

static int line = 0;
static spinlock_t slock = SPINLOCK_INIT;

void _log(const char *format, ...)
{
    va_list list;
    va_start(list);

    _n_log(format, list);

    va_end(list);
}

void _n_log(const char *format, va_list list)
{
    spinlock_acquire(&slock);

    char buf[256] = {0};
    vsnprintf(buf, 256, format, list);

    arch_serial_send_str(buf);
    arch_serial_send_str("\n");

    for (int i = 0; buf[i] != '\0'; i++)
        draw_char(&video_fb, i * font_basic.width, line * font_basic.height, buf[i], &font_basic, 0xFFFFFF);
    line++;

    spinlock_release(&slock);
}
