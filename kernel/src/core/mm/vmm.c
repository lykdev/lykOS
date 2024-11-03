#include "vmm.h"

#include <arch/ptm.h>
#include <core/mm/pmm.h>

#include <utils/assert.h>
#include <utils/hhdm.h>
#include <utils/list.h>
#include <utils/log.h>
#include <utils/limine/requests.h>

#define SEG_INTERSECTS(BASE1, LENGTH1, BASE2, LENGTH2) ((BASE1) < ((BASE2) + (LENGTH2)) && (BASE2) < ((BASE1) + (LENGTH1)))

// GLOBAL DATA

vmm_addr_space_t vmm_kernel_addr_space;

// UTILS

static vmm_seg_t* alloc_segment()
{
    static list_t free_seg_list = LIST_INIT;

    if (list_is_empty(&free_seg_list))
    {
        vmm_seg_t *segments = (vmm_seg_t*)(pmm_alloc(0) + HHDM);
        u64 seg_count = ARCH_PAGE_GRAN / sizeof(vmm_seg_t);

        for (u64 i = 0; i < seg_count; i++)
        {
            segments[i].list_elem = LIST_NODE_INIT;
            list_append(&free_seg_list, &segments[i].list_elem);
        }
    }
    
    list_node_t *node = free_seg_list.head;
    list_remove(&free_seg_list, node);

    return LIST_GET_CONTAINER(node, vmm_seg_t, list_elem);
}

static void insert_seg(vmm_addr_space_t *addr_space, uptr base, u64 len, vmm_seg_type_t type)
{
    // TODO: free the actual segments.

    FOREACH (n, addr_space->segments)
    {
        vmm_seg_t *seg = LIST_GET_CONTAINER(n, vmm_seg_t, list_elem);
        
        if (SEG_INTERSECTS(base, len + 1, seg->base, seg->len + 1))
        {
            if (seg->type == type)
            {
                uptr end =  base + len < seg->base + seg->len ? seg->base + seg->len : base + len; 
                base = base < seg->base ? base : seg->base;
                len  = end - base;

                list_remove(&addr_space->segments, n);
            }
            else
            {
                if (base <= seg->base && base + len >= seg->base + seg->len)
                    list_remove(&addr_space->segments, n);
                else if (base <= seg->base && base + len < seg->base + seg->len)
                {
                    seg->len = seg->base + seg->len - base - len;
                    seg->base = base + len;
                }   
                else if (base > seg->base && base + len >= seg->base + seg->len)
                    seg->len = base - seg->base;
                else
                {
                    vmm_seg_t *new_seg = alloc_segment();
                    *new_seg = (vmm_seg_t) {
                        .base = base + len,
                        .len  = seg->base + seg->len - base - len,
                        .type = seg->type,
                        .list_elem = LIST_NODE_INIT,
                        .addr_space = addr_space
                    };
                    list_insert_after(&addr_space->segments, n, &new_seg->list_elem);

                    seg->len = base - seg->base;
                }
            }
        }
    }

    vmm_seg_t *created_seg = alloc_segment();
    *created_seg = (vmm_seg_t) {
        .base = base,
        .len  = len,
        .type = type,
        .list_elem = LIST_NODE_INIT,
        .addr_space = addr_space
    };

    list_node_t *pos = NULL;
    FOREACH (n, addr_space->segments)
    {
        vmm_seg_t *seg = LIST_GET_CONTAINER(n, vmm_seg_t, list_elem);

        if (seg->base < base)
            pos = n;
    }
    list_insert_after(&addr_space->segments, pos, &created_seg->list_elem);
}

// Actual VMM logic

uptr vmm_map_anon(vmm_addr_space_t *addr_space, uptr virt, size_t len)
{
    ASSERT(virt % ARCH_PAGE_GRAN == 0 and
           len  % ARCH_PAGE_GRAN == 0
    );

    slock_acquire(&addr_space->slock);

    insert_seg(addr_space, virt, len, VMM_ANON);

    for (uptr addr = 0; addr < len; addr += ARCH_PAGE_SIZE_4K)
        arch_ptm_map(&addr_space->ptm_map, virt + addr, (uptr)pmm_alloc(0), ARCH_PAGE_SIZE_4K);

    slock_release(&addr_space->slock);
    return 0;
}

uptr vmm_map_direct(vmm_addr_space_t *addr_space, uptr virt, size_t len, uptr phys)
{
    ASSERT(virt % ARCH_PAGE_GRAN == 0 and
           phys % ARCH_PAGE_GRAN == 0 and
           len  % ARCH_PAGE_GRAN == 0
    );

    slock_acquire(&addr_space->slock);

    insert_seg(addr_space, virt, len, VMM_ANON);

    for (uptr addr = 0; addr < len; addr += ARCH_PAGE_SIZE_4K)
        arch_ptm_map(&addr_space->ptm_map, virt + addr, phys + addr, ARCH_PAGE_SIZE_4K);

    slock_release(&addr_space->slock);
    return 0;
}

vmm_addr_space_t vmm_new_addr_space(uptr limit_low, uptr limit_high)
{
    vmm_addr_space_t addr_space;

    addr_space.slock      = SLOCK_INIT;
    addr_space.segments   = LIST_INIT;
    addr_space.limit_low  = limit_low;
    addr_space.limit_high = limit_high;
    addr_space.ptm_map    = arch_ptm_new_map();

    return addr_space;
}

void vmm_load_addr_space(vmm_addr_space_t *addr_space)
{
    arch_ptm_load_map(&addr_space->ptm_map);
}

void vmm_init()
{
    arch_ptm_init();

    vmm_kernel_addr_space = vmm_new_addr_space(ARCH_HIGHER_HALF_START, ARCH_MAX_VIRT_ADDR);

    vmm_map_direct(&vmm_kernel_addr_space, HHDM, 4 * GIB, 0);
    vmm_map_direct(&vmm_kernel_addr_space, request_kernel_addr.response->virtual_base, 2 * GIB, request_kernel_addr.response->physical_base);

    vmm_load_addr_space(&vmm_kernel_addr_space);

    log("VMM init.");
}
