#include "vmm.h"

#include <arch/ptm.h>

#include <common/assert.h>
#include <common/hhdm.h>
#include <common/limine/requests.h>
#include <common/log.h>
#include <lib/list.h>
#include <lib/math.h>
#include <mm/kmem.h>
#include <mm/pmm.h>

#define SEG_INTERSECTS(BASE1, LENGTH1, BASE2, LENGTH2) ((BASE1) < ((BASE2) + (LENGTH2)) && (BASE2) < ((BASE1) + (LENGTH1)))

vmm_addr_space_t *g_vmm_kernel_addr_space;

static kmem_cache_t *g_segment_cache;

static void vmm_insert_seg(vmm_addr_space_t *addr_space, vmm_seg_t *seg)
{
    slock_acquire(&addr_space->slock);

    list_node_t *pos = NULL;
    FOREACH(n, addr_space->segments)
    {
        vmm_seg_t *i = LIST_GET_CONTAINER(n, vmm_seg_t, list_elem);

        if (i->base < seg->base)
            pos = n;
    }
    list_insert_after(&addr_space->segments, pos, &seg->list_elem);

    slock_release(&addr_space->slock);
}

uptr vmm_find_space(vmm_addr_space_t *addr_space, u64 len)
{
    if (list_is_empty(&addr_space->segments))
        return addr_space->limit_low;

    uptr start;
    FOREACH(n, addr_space->segments)
    {
        vmm_seg_t *seg1 = LIST_GET_CONTAINER(n, vmm_seg_t, list_elem);
        start = seg1->base + seg1->len;

        if (n->next != NULL)
        {
            vmm_seg_t *seg2 = LIST_GET_CONTAINER(n->next, vmm_seg_t, list_elem);

            if (start + len < seg2->base)
                break;
        }
    }

    if (start + len - 1 <= addr_space->limit_high)
        return start;

    return 0;
}

vmm_seg_t *vmm_addr_to_seg(vmm_addr_space_t *addr_space, uptr addr)
{
    FOREACH(n, addr_space->segments)
    {
        vmm_seg_t *seg = LIST_GET_CONTAINER(n, vmm_seg_t, list_elem);

        if (seg->base <= addr && addr < seg->base + seg->len)
            return seg;
    }

    return NULL;
}

bool vmm_pagefault_handler(vmm_addr_space_t *addr_space, uptr addr)
{
    vmm_seg_t *seg = vmm_addr_to_seg(addr_space, addr);

    switch (seg->type)
    {
    case VMM_SEG_ANON:
    {
        uptr virt = FLOOR(addr, ARCH_PAGE_GRAN);
        uptr phys = (uptr)pmm_alloc(0);
        arch_ptm_map(
            &seg->addr_space->ptm_map,
            virt,
            phys,
            ARCH_PAGE_GRAN
        );
        return true;
    }
    case VMM_SEG_FIXED:
    {
        uptr virt = FLOOR(addr, ARCH_PAGE_GRAN);
        uptr phys = seg->off + (virt - seg->base);
        arch_ptm_map(
            &seg->addr_space->ptm_map,
            virt,
            phys,
            ARCH_PAGE_GRAN
        );
        return true;
    }
    default:
        return false;
    }
}

void vmm_map_anon(vmm_addr_space_t *addr_space, uptr virt, u64 len, vmm_prot_t prot)
{
    (void)(prot);
    ASSERT(virt % ARCH_PAGE_GRAN == 0 and len % ARCH_PAGE_GRAN == 0);
    
    vmm_seg_t *created_seg = kmem_alloc_from(g_segment_cache);
    *created_seg = (vmm_seg_t) {
        .addr_space = addr_space,
        .type = VMM_SEG_ANON,
        .base = virt,
        .len  = len,
        .off  = 0,
        .list_elem = LIST_NODE_INIT
    };
    vmm_insert_seg(addr_space, created_seg);    
}

void vmm_map_fixed(vmm_addr_space_t *addr_space, uptr virt, u64 len, vmm_prot_t prot, uptr phys, bool premap)
{
    (void)(prot);
    ASSERT(virt % ARCH_PAGE_GRAN == 0 and len % ARCH_PAGE_GRAN == 0);

    vmm_seg_t *created_seg = kmem_alloc_from(g_segment_cache);
    *created_seg = (vmm_seg_t) {
        .addr_space = addr_space,
        .type = VMM_SEG_FIXED,
        .base = virt,
        .len  = len,
        .off  = phys,
        .list_elem = LIST_NODE_INIT
    };
    vmm_insert_seg(addr_space, created_seg);

    if (premap)
    {
        slock_acquire(&addr_space->slock);

        for (uptr addr = 0; addr < len; addr += ARCH_PAGE_SIZE_4K)
            arch_ptm_map(&addr_space->ptm_map, virt + addr, phys + addr, ARCH_PAGE_SIZE_4K);

        slock_release(&addr_space->slock);
    }
}

uptr vmm_virt_to_phys(vmm_addr_space_t *addr_space, uptr virt)
{
    return arch_ptm_virt_to_phys(&addr_space->ptm_map, virt);
}

vmm_addr_space_t *vmm_new_addr_space(uptr limit_low, uptr limit_high)
{
    vmm_addr_space_t *addr_space = kmem_alloc(sizeof(vmm_addr_space_t));

    *addr_space = (vmm_addr_space_t) {
        .slock = SLOCK_INIT,
        .segments = LIST_INIT,
        .limit_low = limit_low,
        .limit_high = limit_high, 
        .ptm_map = arch_ptm_new_map()
    };
    return addr_space;
}

void vmm_load_addr_space(vmm_addr_space_t *addr_space)
{
    arch_ptm_load_map(&addr_space->ptm_map);
}

void vmm_init()
{
    arch_ptm_init();

    g_vmm_kernel_addr_space = vmm_new_addr_space(ARCH_HIGHER_HALF_START, ARCH_MAX_VIRT_ADDR);

    g_segment_cache = kmem_new_cache("VMM Segment Cache", sizeof(vmm_seg_t));

    // Mappings done according to the Limine specification.
    vmm_map_fixed(
        g_vmm_kernel_addr_space,
        HHDM,
        4 * GIB,
        VMM_FULL,
        0,
        true
    );
    vmm_map_fixed(
        g_vmm_kernel_addr_space,
        request_kernel_addr.response->virtual_base,
        2 * GIB,
        VMM_FULL,
        request_kernel_addr.response->physical_base,
        true // Premap this segment. PF handlers are found here so we cannot rely on them to do the mapping later.
    );

    vmm_load_addr_space(g_vmm_kernel_addr_space);

    log("VMM initialized.");
}
