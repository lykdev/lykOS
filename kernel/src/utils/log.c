#include "log.h"

#include <arch/serial.h>
#include <graphics/draw.h>
#include <graphics/font_basic.h>

#include <utils/printf.h>
#include <utils/slock.h>
#include <utils/string.h>

static int line = 0;
static slock_t slock = SLOCK_INIT;

void _log(const char *file, const char *format, ...)
{
    va_list list;
    va_start(list);

    _n_log(file, format, list);
    
    va_end(list);    
}

void _n_log(const char *file, const char *format, va_list list)
{
    slock_acquire(&slock);

    char buf[256];
    int offset = sprintf(buf, "[%s] ", strrchr(file, '/') + 1);
    vsnprintf(&buf[offset], 256, format, list);

    #if defined (__x86_64__)
        arch_serial_send_str(buf);
        arch_serial_send_str("\n");
    #endif

    if (line > 45)
    {
        slock_release(&slock);
        return;
    }

    for (int i = 0; buf[i] != '\0'; i++)
        draw_char(&video_fb, i * font_basic.width, line * font_basic.height, buf[i], &font_basic, 0xFFFFFF);
    line++;

    slock_release(&slock);
}
