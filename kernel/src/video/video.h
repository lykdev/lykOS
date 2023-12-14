#pragma once

#pragma once
#include <lib/utils.h>

u32 *video_buffer;
u64 video_width;
u64 video_height;
u64 video_pitch;

void video_init();
void video_set_pixel(u64 x, u64 y, u32 color);
void video_draw_str(u64 x, u64 y, u32 color, char *str);