#include "block.h"

#include <common/panic.h>
#include <fs/gpt.h>
#include <fs/mbr.h>
#include <lib/errno.h>
#include <lib/string.h>
#include <mm/heap.h>

#define BLKRRPART        0x1000 // Re-read partition table.
#define BLKGETSIZE       0x1001
#define BLKGETSIZE64     0x1002
#define BLKSSZGET        0x1003

static list_t g_drive_list = LIST_INIT;
static spinlock_t g_slock = SPINLOCK_INIT;

static int block_device_ioctl(block_device_t *self, u64 request, void *args);

static void add_partition(block_device_t *dev, char *name, u64 lba_offset, u64 sector_count)
{
    block_device_t *part = heap_alloc(sizeof(block_device_t));
    *dev = (block_device_t) {
        .name = strdup(name),
        .type = BLOCK_DEVICE_DRIVE,
        .partitions = LIST_INIT,
        .parent = dev,
        .lba_offset = lba_offset,
        .sector_size = dev->sector_size,
        .sector_count = sector_count,
        .read = dev->read,
        .write = dev->write,
        .ioctl = block_device_ioctl,
        .driver_data = dev->driver_data,
        .slock = SPINLOCK_INIT,
        .list_node = LIST_NODE_INIT,
        .ref_count = 0
    };

    list_append(&dev->partitions, &part->list_node);
}

static int read_part_table(block_device_t *dev)
{
    if (dev->type != BLOCK_DEVICE_DRIVE)
        return -1;

    if (!dev->read || !dev->write || !dev->ioctl)
        panic("Registered storage device does not implement all required fields!");

    bool gpt_disk = false;

    mbr_header_t *mbr_hdr = heap_alloc(dev->sector_size);
    dev->read(dev, 0, &mbr_hdr, 1);

    if (mbr_hdr->signature == 0xAA55)
        for (uint i = 0; i < 4; i++)
        {
            mbr_part_table_entry_t *p = &mbr_hdr->part_table[i];
            if (p->type == 0xEE) // Protective MBR.
            {   gpt_disk = true;
                break;
            }
            else if (p->lba_start && p->sector_count)
                add_partition(dev, NULL, p->lba_start, p->sector_count);
        }

    if (!gpt_disk)
        return EOK;

    gpt_header_t *gpt_hdr = heap_alloc(dev->sector_size);
    dev->read(dev, 1, &gpt_hdr, 1);

    if (memcmp(gpt_hdr->signature, "EFI PART", 8) == 0)
    {
        u64 entries_size = gpt_hdr->part_table_entry_count * gpt_hdr->part_table_entry_size;
        void *entries = heap_alloc(entries_size);
        dev->read(dev, gpt_hdr->part_table_lba, entries,
                    entries_size / dev->sector_size);

        for (uint i = 0; i < gpt_hdr->part_table_entry_count; i++)
        {
            gpt_part_table_entry_t *p = (gpt_part_table_entry_t*)((uptr)entries + i * gpt_hdr->part_table_entry_size);
            if (p->lba_start && p->lba_end)
                add_partition(dev, (char *)p->partition_name, p->lba_start, p->lba_end - p->lba_start + 1);
        }
    }

    return EOK;
}

static int block_device_ioctl(block_device_t *self, u64 request, void *args)
{
    switch (request)
    {
    case BLKRRPART:
        return read_part_table(self);
    case BLKGETSIZE:
        if (!args) return -1;
        *(u32 *)args = (u32)self->sector_count;
        return EOK;
    case BLKGETSIZE64:
        if (!args) return -1;
        *(u64 *)args = self->sector_count * self->sector_size;
        return EOK;
    case BLKSSZGET:
        if (!args) return -1;
        *(u32 *)args = (u32)self->sector_size;
        return EOK;
    default:
        return EINVAL;
    }
}

void block_device_register_drive(
    const char *name,
    u64 sector_size,
    u64 sector_count,
    block_device_read_t read,
    block_device_write_t write,
    void *driver_data
)
{
    block_device_t *dev = heap_alloc(sizeof(block_device_t));
    *dev = (block_device_t) {
        .name = strdup(name),
        .type = BLOCK_DEVICE_DRIVE,
        .partitions = LIST_INIT,
        .parent = NULL,
        .lba_offset = 0,
        .sector_size = sector_size,
        .sector_count = sector_count,
        .read = read,
        .write = write,
        .ioctl = block_device_ioctl,
        .driver_data = driver_data,
        .slock = SPINLOCK_INIT,
        .list_node = LIST_NODE_INIT,
        .ref_count = 0
    };

    spinlock_acquire(&g_slock);
    list_append(&g_drive_list, &dev->list_node);
    spinlock_release(&g_slock);
}
