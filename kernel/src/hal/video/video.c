#include "video.h"

#include <lib/utils.h>
#include "font_8x8.h"

void video_init()
{
    ASSERT(framebuffer_request.response != NULL &&
           framebuffer_request.response->framebuffer_count > 0);

    video_buffer = (u32*)framebuffer_request.response->framebuffers[0]->address;
    video_width = framebuffer_request.response->framebuffers[0]->width;
    video_height = framebuffer_request.response->framebuffers[0]->height;
    video_pitch = framebuffer_request.response->framebuffers[0]->pitch;

    video_draw_str(500, 50, 0x986DAD, "Video test: ABC Test 12~!@#$%^&*()\\");
}

void video_set_pixel(u64 x, u64 y, u32 color)
{
    video_buffer[y * video_pitch + x] = color;
}

static void draw_char(u64 x, u64 y, u32 color, char ch)
{
    u64 offset = y * (video_pitch / 4) + x;

    for (int j = 0; j < 8; j++)
    {
        for (int i = 0; i < 8; i++)
            if ((font_8x8[(u64)ch][j] >> i) & 1)
                video_buffer[offset + i] = color;
        offset += video_pitch / 4;
    }        
}

void video_draw_str(u64 x, u64 y, u32 color, char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
        draw_char(x + i * 8, y, color, str[i]);
}