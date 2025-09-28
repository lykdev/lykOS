#include "storage.h"
#include "common/sync/spinlock.h"

#include <common/panic.h>
#include <fs/gpt.h>
#include <fs/mbr.h>
#include <lib/errno.h>
#include <lib/string.h>
#include <mm/heap.h>

list_t g_storage_device_list = LIST_INIT;
spinlock_t g_storage_device_slock = SPINLOCK_INIT;

static void add_partition(storage_device_t *dev, char *name, u64 lba_offset, u64 sector_count)
{
    storage_partition_t *part = heap_alloc(sizeof(storage_device_t));
    *part = (storage_partition_t) {
        .name = strdup(name),
        .parent = dev,
        .sector_offset = lba_offset,
        .sector_count = sector_count,
        .slock = SPINLOCK_INIT,
        .list_node = LIST_NODE_INIT,
        .ref_count = 1
    };

    list_append(&dev->partitions, &part->list_node);
}

bool storage_scan_partitions(storage_device_t *dev)
{
    spinlock_acquire(&dev->slock);

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
    {
        spinlock_release(&dev->slock);
        return EOK;
    }

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

    spinlock_release(&dev->slock);
    return EOK;
}
