#include "video.h"

#include <lib/req.h>
#include <lib/log.h>

framebuffer_t video_early_fb;

void video_init()
{
    struct limine_framebuffer *b = req_framebuffer.response->framebuffers[0];

    video_early_fb.addr   = (uptr)b->address;
    video_early_fb.width  = b->width;
    video_early_fb.height = b->height;
    video_early_fb.pitch  = b->pitch;

    log("Early framebuffer info: %ux%u", video_early_fb.width, video_early_fb.height);
}
