
#include <common/hhdm.h>
#include <common/log.h>
#include <common/panic.h>
#include <fs/vfs.h>
#include <graphics/video.h>
#include <lib/def.h>
#include <lib/string.h>

static int fb_write(vnode_t *self, u64 offset, void *buffer, u64 count, u64 *out)
{
    if (offset + count > self->size)
        count = self->size;

    memcpy((void*)(video_fb.addr + offset), buffer, count);

    *out = count;
    return EOK;
}

vnode_ops_t char_ops = {
    .write = fb_write
};

void dev_fb_init()
{
    vnode_t *char_fb;
    if (vfs_create("/dev/fb", VFS_NODE_CHAR, &char_fb) < 0)
        panic("Could not create file `/dev/fb`.");

    char_fb->size = video_fb.size;
    char_fb->ops = &char_ops;
}
