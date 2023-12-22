#include "pmm.h"

#include <lib/slock.h>
#include <lib/utils.h>

static u64 *page_list = NULL;
static slock lock = SLOCK_INIT;

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

void pmm_free(void *page)
{
    slock_acquire(&lock);

    if ((u64)page > HHDM)
        panic("Don't pmm_free a higher half address.");

    *(u64*)(page + HHDM) = (u64)page_list;
    page_list = (u64*)page;

    used_ram -= PAGE_SIZE;
    free_ram += PAGE_SIZE;

    slock_release(&lock);
}

void* pmm_alloc()
{
    slock_acquire(&lock);

    void *ret = (void*)page_list;
    // We will not check to see if ret is null. In some cases we can still recover from this.

    page_list = (u64*)*((u64*)((uptr)page_list + HHDM)); // Set the top of the page list to point to the next element in the list.

    used_ram += PAGE_SIZE;
    free_ram -= PAGE_SIZE;

    slock_release(&lock);
    
    return ret;
}


void pmm_init()
{
    ASSERT_C(memmap_request.response != NULL && memmap_request.response->entry_count > 0,
             "Invalid memory map provided by the bootloader.");

    for (u64 i = 0; i < memmap_request.response->entry_count; i++)
    {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];

        log("e %d - 0x%llx, 0x%llx, %lluKiB", entry->type, entry->base, entry->base + entry->length, entry->length / KIB);

        if (entry->type == LIMINE_MEMMAP_USABLE)
        {           
            for (uptr addr = entry->base + entry->length - PAGE_SIZE; addr >= entry->base; addr -= PAGE_SIZE)
            {                
                pmm_free((void*)addr);
                total_ram += PAGE_SIZE;
            }
        }
    }

    log("System total usable RAM: %dMiB", total_ram / MIB);
}