#include "pmm.h"

#include <lib/utils.h>

static u64 *page_list = NULL;

static u64 total_ram = 0;
static u64 used_ram = 0;
static u64 free_ram = 0;

u64 pmm_get_total_ram()
{
    return total_ram;
}

u64 pmm_get_used_ram()
{
    return used_ram;
}

u64 pmm_get_free_ram()
{
    return free_ram;
}

void pmm_free_page(void *page)
{
    *(u64*)page = (u64)page_list;
    page_list = (u64*)page;

    used_ram -= PAGE_SIZE;
    free_ram += PAGE_SIZE;
}

void* pmm_get_page()
{
    void *ret = (void*)page_list;
    // We will not check to see if ret is null. In some cases we can still recover from this.

    page_list = (u64*)*page_list;

    used_ram += PAGE_SIZE;
    free_ram -= PAGE_SIZE;

    //log("%llx", ret);
    return ret;
}

void pmm_init()
{
    ASSERT_C(memmap_request.response != NULL && memmap_request.response->entry_count > 0,
             "Invalid memory map provided by the bootloader.");

    for (u64 i = 0; i < memmap_request.response->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE)
        {
            for (uptr addr = entry->base + entry->length - PAGE_SIZE; addr >= entry->base; addr -= PAGE_SIZE)
            {                
                pmm_free_page((void*)(addr + HHDM));
                total_ram += PAGE_SIZE;
            }
        }
    }

    log("System total usable RAM: %dMiB", total_ram / MIB);
}