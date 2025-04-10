
// #include <common/hhdm.h>
// #include <fs/vfs.h>
// #include <graphics/video.h>
// #include <lib/def.h>
// #include <lib/string.h>

// static int fb_write(vfs_node_t *self, u64 offset, void *buffer, u64 count)
// {  
//     if (offset + count > self->size)
//         count = self->size - offset;
//     memcpy((void*)(video_fb.addr + offset), buffer, count);

//     return count;
// }

// void dev_fb_init()
// {
//     vfs_node_t *dev;
//     vfs_lookup("/dev", &dev);
//     vfs_node_t *fb;
//     dev->ops->crate(dev, VFS_NODE_CHAR, "fb", &fb);

//     fb->ops = (vfs_node_ops_t) {
//         .read = NULL,
//         .write = fb_write,
//         .ioctl = NULL
//     };
// }