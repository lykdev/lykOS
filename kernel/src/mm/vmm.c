#include "vmm.h"

#include <arch/types.h>
#include <arch/ptm.h>
#include <common/assert.h>
#include <common/hhdm.h>
#include <common/limine/requests.h>
#include <common/log.h>
#include <common/sync/spinlock.h>
#include <lib/list.h>
#include <lib/math.h>
#include <lib/string.h>
#include <mm/kmem.h>
#include <mm/heap.h>
#include <mm/pmm.h>

typedef enum
{
    VMM_SEG_VNODE,
    VMM_SEG_KERNEL
}
vmm_seg_type_t;

typedef struct
{
    vmm_seg_type_t type;
    uptr base;
    u64 len;
    //
    int prot, flags;
    //
    vfs_node_t *vnode;
    uptr offset;
    //
    list_node_t list_node;
}
vmm_seg_t;

vmm_addr_space_t *g_vmm_kernel_addr_space;
static kmem_cache_t *g_segment_cache;

static void vmm_insert_seg(vmm_addr_space_t *as, vmm_seg_t *seg)
{
    spinlock_acquire(&as->slock);

    list_node_t *pos = NULL;
    FOREACH(n, as->segments)
    {
        vmm_seg_t *i = LIST_GET_CONTAINER(n, vmm_seg_t, list_node);

        if (i->base < seg->base)
            pos = n;
        else
            break; // Given that the list is sorted, an earlier position must have been found.
    }

    if (pos)
        list_insert_after(&as->segments, pos, &seg->list_node);
    else
        list_prepend(&as->segments, &seg->list_node);

    spinlock_release(&as->slock);
}

static uptr vmm_find_space(vmm_addr_space_t *as, u64 len)
{
    if (list_is_empty(&as->segments))
        return as->limit_low;

    uptr start = as->limit_low;
    FOREACH(n, as->segments)
    {
        vmm_seg_t *seg = LIST_GET_CONTAINER(n, vmm_seg_t, list_node);
        // If there's enough space between current start and this segment.
        if (start + len < seg->base)
            break;
        // Update start to point to the end of this segment.
        start = seg->base + seg->len;
    }

    // Check if there us space after the last segment.
    if (start + len - 1 <= as->limit_high)
        return start;

    return 0;
}

static bool vmm_check_collision(vmm_addr_space_t *as, uptr base, u64 len)
{
    uptr end = base + len - 1;

    FOREACH(n, as->segments)
    {
        vmm_seg_t *seg = LIST_GET_CONTAINER(n, vmm_seg_t, list_node);
        uptr seg_base = seg->base;
        uptr seg_end = seg->base + seg->len - 1;

        if (end >= seg_base && base <= seg_end)
            return true;
    }

    return false;
}

static bool vmm_is_in_bounds(vmm_addr_space_t *as, uptr base, u64 len)
{
    if (base < as->limit_low || base + len > as->limit_high)
        return false;

    return true;
}

vmm_seg_t *vmm_addr_to_seg(vmm_addr_space_t *as, uptr addr)
{
    FOREACH (n, as->segments)
    {
        vmm_seg_t *seg = LIST_GET_CONTAINER(n, vmm_seg_t, list_node);

        if (seg->base <= addr && addr < seg->base + seg->len)
            return seg;
    }

    return NULL;
}

bool vmm_pagefault_handler(vmm_addr_space_t *as, uptr addr)
{
    vmm_seg_t *seg = vmm_addr_to_seg(as, addr);
    if (seg == NULL)
    {
        log("VMM: bruh");
        return false;
    }

    if (seg->type == VMM_SEG_KERNEL)
        panic("PF on kernel segment!");

    uptr virt = FLOOR(addr, ARCH_PAGE_GRAN);
    uptr phys = (uptr)pmm_alloc(0);
    arch_ptm_map(
        &as->ptm_map,
        virt,
        phys,
        ARCH_PAGE_GRAN
    );

    if (seg->flags & VMM_MAP_ANON)
        memset((void*)(phys + HHDM), 0, ARCH_PAGE_GRAN);
    else
    {
        u64 count = seg->vnode->file_ops->read(seg->vnode, addr - seg->base + seg->offset, (void*)(phys + HHDM), ARCH_PAGE_GRAN);
        ASSERT(count == ARCH_PAGE_GRAN);
    }

    return true;
}

void *vmm_map_vnode(vmm_addr_space_t *as, uptr virt, u64 len, int prot, int flags, vfs_node_t *vnode, u64 offset)
{
    ASSERT(virt % ARCH_PAGE_GRAN == 0 && len % ARCH_PAGE_GRAN == 0);
    ASSERT(((flags & VMM_MAP_PRIVATE) != 0) ^ ((flags & VMM_MAP_SHARED) != 0));

    if (flags & VMM_MAP_ANON)
        vnode = NULL, offset = 0;
    else
        ASSERT(vnode != NULL);

    if (vmm_check_collision(as, virt, len) || !vmm_is_in_bounds(as, virt, len))
    {
        if (flags & VMM_MAP_FIXED)
            return VMM_MAP_FAILED;
        else
            virt = vmm_find_space(as, len);
    }

    if (flags & VMM_MAP_POPULATE)
    {
        for (uptr addr = 0; addr < len; addr += ARCH_PAGE_GRAN)
        {
            uptr phys = (uptr)pmm_alloc(0);
            arch_ptm_map(&as->ptm_map, virt + addr, phys, ARCH_PAGE_GRAN);

            if (flags & VMM_MAP_ANON)
                memset((void*)(phys + HHDM), 0, ARCH_PAGE_GRAN);
            else
                vnode->file_ops->read(vnode, offset + addr, (void*)(phys + HHDM), ARCH_PAGE_GRAN);
        }
    }

    vmm_seg_t *created_seg = kmem_alloc_cache(g_segment_cache);
    *created_seg = (vmm_seg_t) {
        .type = VMM_SEG_VNODE,
        .base = virt,
        .len = len,
        .prot = prot,
        .flags = flags,
        .vnode = vnode,
        .offset = offset,
        .list_node = LIST_NODE_INIT
    };
    vmm_insert_seg(as, created_seg);

    return (void*)virt;
}

void vmm_map_kernel(vmm_addr_space_t *as, uptr virt, u64 len, int prot, uptr phys)
{
    (void)(prot);
    ASSERT(virt % ARCH_PAGE_GRAN == 0 && len % ARCH_PAGE_GRAN == 0);

    vmm_seg_t *created_seg = kmem_alloc_cache(g_segment_cache);
    *created_seg = (vmm_seg_t) {
        .type = VMM_SEG_KERNEL,
        .base = virt,
        .len = len,
        .prot = prot,
        .list_node = LIST_NODE_INIT
    };
    vmm_insert_seg(as, created_seg);

    spinlock_acquire(&as->slock);

    for (uptr addr = 0; addr < len; addr += ARCH_PAGE_SIZE_4K)
        arch_ptm_map(&as->ptm_map, virt + addr, phys + addr, ARCH_PAGE_SIZE_4K);

    spinlock_release(&as->slock);
}

uptr vmm_virt_to_phys(vmm_addr_space_t *as, uptr virt)
{
    return arch_ptm_virt_to_phys(&as->ptm_map, virt);
}

vmm_addr_space_t *vmm_new_addr_space(uptr limit_low, uptr limit_high)
{
    vmm_addr_space_t *addr_space = heap_alloc(sizeof(vmm_addr_space_t));

    *addr_space = (vmm_addr_space_t) {
        .slock = SPINLOCK_INIT,
        .segments = LIST_INIT,
        .limit_low = limit_low,
        .limit_high = limit_high,
        .ptm_map = arch_ptm_new_map()
    };
    return addr_space;
}

void vmm_load_addr_space(vmm_addr_space_t *as)
{
    arch_ptm_load_map(&as->ptm_map);
}

void vmm_init()
{
    if (request_memmap.response == NULL)
        panic("Invalid memory map provided by bootloader!");

    arch_ptm_init();

    g_segment_cache = kmem_new_cache("VMM Segment Cache", sizeof(vmm_seg_t));

    g_vmm_kernel_addr_space = vmm_new_addr_space(ARCH_KERNEL_MIN_VIRT, ARCH_KERNEL_MAX_VIRT);

    // Mappings done according to the Limine specification.
    vmm_map_kernel(
        g_vmm_kernel_addr_space,
        HHDM,
        4 * GIB,
        VMM_PROT_FULL,
        0
    );
    vmm_map_kernel(
        g_vmm_kernel_addr_space,
        request_kernel_addr.response->virtual_base,
        2 * GIB,
        VMM_PROT_FULL,
        request_kernel_addr.response->physical_base
    );
    for (u64 i = 0; i < request_memmap.response->entry_count; i++)
    {
        struct limine_memmap_entry *e = request_memmap.response->entries[i];
        if (e->type == LIMINE_MEMMAP_RESERVED ||
            e->type == LIMINE_MEMMAP_BAD_MEMORY)
            continue;

        uptr base   = FLOOR(e->base, ARCH_PAGE_GRAN);
        u64  length = CEIL(e->base + e->length, ARCH_PAGE_GRAN) - base;

        vmm_map_kernel(
            g_vmm_kernel_addr_space,
            base + HHDM,
            length,
            VMM_PROT_FULL,
            base
        );
    }

    vmm_load_addr_space(g_vmm_kernel_addr_space);

    log("VMM initialized.");
}

u64 vmm_copy_to(vmm_addr_space_t *dest_as, uptr dest_addr, void *src, u64 count)
{
    u64 i = 0;
    while (i < count)
    {
        u64 offset = (dest_addr + i) % ARCH_PAGE_GRAN;
        uptr phys = vmm_virt_to_phys(dest_as, dest_addr + i);
        if (phys == BAD_ADDRESS)
        {
            vmm_pagefault_handler(dest_as, dest_addr + i);
            phys = vmm_virt_to_phys(dest_as, dest_addr + i);
            if (phys == BAD_ADDRESS)
                panic("tf");
        }

        u64 len = MIN(count - i, ARCH_PAGE_GRAN - offset);
        memcpy((void*)(phys + HHDM), src, len);
        i += len;
        src += len;
    }
    return i;
}

u64 vmm_zero_out(vmm_addr_space_t *dest_as, uptr dest_addr, u64 count)
{
    u64 i = 0;
    while (i < count)
    {
        u64 offset = (dest_addr + i) % ARCH_PAGE_GRAN;
        uptr phys = vmm_virt_to_phys(dest_as, dest_addr + i);
        if (phys == BAD_ADDRESS)
        {
            vmm_pagefault_handler(dest_as, dest_addr + i);
            phys = vmm_virt_to_phys(dest_as, dest_addr + i);
            if (phys == BAD_ADDRESS)
                panic("tf");
        }

        u64 len = MIN(count - i, ARCH_PAGE_GRAN - offset);
        memset((void*)(phys + HHDM), 0, len);
        i += len;
    }
    return i;
}
