#include "storage.h"

#include <common/panic.h>
#include <fs/gpt.h>
#include <fs/mbr.h>
#include <lib/string.h>
#include <mm/heap.h>

static list_t g_device_list = LIST_INIT;
static spinlock_t g_slock = SPINLOCK_INIT;

static void add_partition(storage_device_t *dev, u16 *name, int num, u64 lba_start, u64 sector_count)
{
    partition_t *p = heap_alloc(sizeof(partition_t));
    if (name)
        memcpy(p->name, name, 72);
    else
        memset(p->name, 0, 72);
    p->number = num,
    p->lba_start = lba_start,
    p->sector_count = sector_count,

    list_append(&dev->partitions, &p->list_node);
}

void storage_device_register(storage_device_t *dev)
{
    spinlock_acquire(&g_slock);

    if (!dev->lba_size || !dev->read || !dev->write)
        panic("Registered storage device does not implement all required fields!");

    bool gpt_disk = false;

    mbr_header_t *mbr_hdr = heap_alloc(dev->lba_size);
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
                add_partition(dev, NULL, i, p->lba_start, p->sector_count);
        }

    if (!gpt_disk)
        goto skip_gpt;

    gpt_header_t *gpt_hdr = heap_alloc(dev->lba_size);
    dev->read(dev, 1, &gpt_hdr, 1);

    if (memcmp(gpt_hdr->signature, "EFI PART", 8) == 0)
    {
        u64 entries_size = gpt_hdr->part_table_entry_count * gpt_hdr->part_table_entry_size;
        void *entries = heap_alloc(entries_size);
        dev->read(dev, gpt_hdr->part_table_lba, entries,
                  entries_size / dev->lba_size);

        for (uint i = 0; i < gpt_hdr->part_table_entry_count; i++)
        {
            gpt_part_table_entry_t *p = (gpt_part_table_entry_t*)((uptr)entries + i * gpt_hdr->part_table_entry_size);
            if (p->lba_start && p->lba_end)
                add_partition(dev, p->partition_name, i, p->lba_start, p->lba_end - p->lba_start + 1);
        }
    }

skip_gpt:

    list_append(&g_device_list, &dev->list_node);
    spinlock_release(&g_slock);
}
