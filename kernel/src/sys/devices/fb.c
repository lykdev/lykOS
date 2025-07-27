
#include <common/hhdm.h>
#include <fs/vfs.h>
#include <graphics/video.h>
#include <lib/def.h>
#include <lib/string.h>
#include <common/log.h>

static u64 fb_write(vfs_node_t *self, u64 offset, void *buffer, u64 count)
{
    if (offset + count > self->size)
        count = self->size;

    memcpy((void*)(video_fb.addr + offset), buffer, count);

    return count;
}

vfs_node_ops_t char_ops = {
    .write = fb_write
};

void dev_fb_init()
{
    vfs_node_t *dev_dir = vfs_lookup("/dev");
    vfs_node_t *char_fb = dev_dir->ops->create(dev_dir, VFS_NODE_CHAR, "fb");

    char_fb->size = video_fb.size;
    char_fb->ops = &char_ops;
}
