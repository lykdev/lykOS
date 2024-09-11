#pragma once

#include <utils/def.h>

typedef struct
{
    uptr addr;
    u64  height;
    u64  width;
    u64  pitch;
} framebuffer_t;

extern framebuffer_t video_fb;

void video_init();