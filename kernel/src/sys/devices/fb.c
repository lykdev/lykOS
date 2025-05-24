
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

static vfs_node_ops_char_t g_node_ops = (vfs_node_ops_char_t) {
    .write = fb_write
};

void dev_fb_init()
{
    vfs_node_t *dev = vfs_lookup("/dev");
    vfs_node_t *fb = dev->dir_ops->create(dev, VFS_NODE_CHAR, "fb");
    fb->size = video_fb.size;
    fb->ops = &g_node_ops;
}
