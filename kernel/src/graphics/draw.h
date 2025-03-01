#pragma once

#include <graphics/font.h>
#include <graphics/video.h>
#include <lib/def.h>

typedef u32 color_t;

void draw_pixel(framebuffer_t *fb, u16 x, u16 y, color_t color);

void draw_char(framebuffer_t *fb, u16 x, u16 y, char c, font_t *font,
               color_t color);
