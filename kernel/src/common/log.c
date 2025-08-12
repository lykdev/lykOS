#include "log.h"

#include <arch/serial.h>
#include <common/sync/spinlock.h>
#include <graphics/draw.h>
#include <graphics/font_basic.h>
#include <lib/list.h>
#include <lib/printf.h>
#include <lib/string.h>

static u64 g_line = 0;
static spinlock_t g_slock = SPINLOCK_INIT;

void _log(const char *format, ...)
{
    va_list list;
    va_start(list);

    _n_log(format, list);

    va_end(list);
}

void _n_log(const char *format, va_list list)
{
    spinlock_acquire(&g_slock);

    char buf[256] = {0};
    vsnprintf(buf, 256, format, list);

    arch_serial_send_str(buf);
    if (buf[strlen(buf) - 1] != '\n')
        arch_serial_send_str("\n");

    // TODO: scroll
    if (g_line < video_fb.height / font_basic.height)
    {
        for (int i = 0; buf[i] != '\0'; i++)
            draw_char(&video_fb, i * font_basic.width, g_line * font_basic.height, buf[i], &font_basic, 0xFFFFFF);
        g_line++;
    }

    spinlock_release(&g_slock);
}
