#include "log.h"

#include <core/graphics/draw.h>
#include <core/graphics/font_basic.h>

#include <utils/printf.h>
#include <utils/slock.h>

static int line = 0;
static slock_t slock = SLOCK_INIT;

void _log(const char *module, int level, const char *format, ...)
{
    if (line > 60)
        return;

    va_list list;
    va_start(list);
    slock_acquire(&slock);

    char buf[256] = {0};
    vsnprintf(buf, 256, format, list);

    for (int i = 0; buf[i] != '\0'; i++)
        draw_char(&video_fb, i * font_basic.width, line * font_basic.height, buf[i], &font_basic, 0xFFFFFF);
    line++;

    slock_release(&slock);
    va_end(list);    
}
