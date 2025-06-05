#include "draw.h"

void draw_pixel(framebuffer_t *fb, u64 x, u64 y, color_t color)
{
    if (x >= fb->width || y >= fb->height)
        return;

    *(u32 *)(fb->addr + y * fb->pitch + x * sizeof(color_t)) = color;
}

void draw_char(framebuffer_t *fb, u64 x, u64 y, char c, font_t *font, color_t color)
{
    for (u8 _y = 0; _y < font->height; _y++)
        for (u8 _x = 0; _x < font->width; _x++)
            if ((font->data[(u64)c * font->height + _y] >> (font->width - _x - 1)) & 1)
                draw_pixel(fb, x + _x, y + _y, color);
}
