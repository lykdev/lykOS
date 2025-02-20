#include "video.h"

#include <utils/limine/requests.h>

framebuffer_t video_fb;

void video_init()
{
    struct limine_framebuffer *b = request_framebuffer.response->framebuffers[0];

    video_fb.addr   = (uptr)b->address;
    video_fb.width  = b->width;
    video_fb.height = b->height;
    video_fb.pitch  = b->pitch;
}
