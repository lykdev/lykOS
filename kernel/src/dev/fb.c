#include <common/log.h>
#include <common/panic.h>
#include <dev/devman.h>
#include <fs/vfs.h>
#include <graphics/video.h>
#include <lib/def.h>
#include <lib/string.h>
#include <mm/heap.h>

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
    driver_t *drv = heap_alloc(sizeof(driver_t));
    *drv = (driver_t) {
        .name = heap_alloc(32),
        .match_data = NULL,
        .list_node = LIST_NODE_INIT
    };
    strcpy((char *)drv->name, "UEFI GOP Driver");
    devman_reg_driver(devman_get_bus_type("OTHER"), drv);

    device_t *dev = heap_alloc(sizeof(device_t));
    *dev = (device_t) {
        .name = heap_alloc(32),
        .data = NULL,
        .driver = drv,
        .driver_data = NULL,
        .list_node = LIST_NODE_INIT
    };
    strcpy((char *)dev->name, "UEFI GOP Framebuffer");
    devman_reg_device(devman_get_bus_type("OTHER"), dev);
}
