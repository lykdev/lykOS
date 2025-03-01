#include "draw.h"

void draw_pixel(framebuffer_t *fb, u16 x, u16 y, color_t color)
{
  if (x < 0 || x >= fb->width || y < 0 || y >= fb->height)
    return;

  *(u32 *)(fb->addr + y * fb->pitch + x * sizeof(color_t)) = color;
}

void draw_char(framebuffer_t *fb, u16 x, u16 y, char c, font_t *font,
               color_t color)
{
  for (u16 _y = 0; _y < font->height; _y++)
    for (u16 _x = 0; _x < font->width; _x++)
      if ((font->data[(u8)c * font->height + _y] >> (font->width - _x - 1)) & 1)
        draw_pixel(fb, x + _x, y + _y, color);
}
