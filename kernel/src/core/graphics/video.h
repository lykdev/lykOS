#pragma once

#include <lib/def.h>
#include <lib/list.h>

typedef struct
{
    uptr addr;
    u64  height;
    u64  width;
    u64  pitch;
} framebuffer_t;

extern framebuffer_t video_early_fb;

void video_init();