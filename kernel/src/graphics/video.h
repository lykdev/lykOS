#pragma once

#include <lib/def.h>

typedef struct
{
    uptr addr;
    u64  height;
    u64  width;
    u64  pitch;
    u64  size;
} framebuffer_t;

extern framebuffer_t video_fb;

void video_init();