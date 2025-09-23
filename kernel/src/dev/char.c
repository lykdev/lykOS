#include "char.h"

#include <common/panic.h>
#include <fs/gpt.h>
#include <fs/mbr.h>
#include <lib/errno.h>
#include <lib/string.h>
#include <mm/heap.h>

static list_t g_char_list = LIST_INIT;
static spinlock_t g_slock = SPINLOCK_INIT;

static int char_device_ioctl(char_device_t *self, u64 request, void *args)
{
    return -1;
}

void char_device_register_drive(
    const char *name,
    char_device_read_t read,
    char_device_write_t write,
    void *driver_data
)
{
    char_device_t *dev = heap_alloc(sizeof(char_device_t));
    *dev = (char_device_t) {
        .name = strdup(name),
        .read = read,
        .write = write,
        .ioctl = char_device_ioctl,
        .driver_data = driver_data,
        .slock = SPINLOCK_INIT,
        .list_node = LIST_NODE_INIT,
        .ref_count = 0
    };

    spinlock_acquire(&g_slock);
    list_append(&g_char_list, &dev->list_node);
    spinlock_release(&g_slock);
}
