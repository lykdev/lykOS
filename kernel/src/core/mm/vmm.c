#include "vmm.h"

#include <core/mm/pmm.h>
#include <utils/hhdm.h>
#include <utils/list.h>
#include <utils/log.h>

#define SEG_INTERSECTS(BASE1, LENGTH1, BASE2, LENGTH2) ((BASE1) < ((BASE2) + (LENGTH2)) && (BASE2) < ((BASE1) + (LENGTH1)))

#define MAX_VIRT_ADDR ((u64)1 << 48 - 1)

static list_t free_seg_list = LIST_INIT;

static vmm_seg_t* alloc_segment()
{
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

static void vmm_map_seg(vmm_addr_space_t *addr_space, uptr base, u64 len, vmm_seg_type_t type)
{
    // TODO: free the actual segments.

    FOREACH (n, addr_space->segments)
    {
        vmm_seg_t *seg = LIST_GET_CONTAINER(n, vmm_seg_t, list_elem);
        
        if (SEG_INTERSECTS(base, len, seg->base, seg->len))
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

void* vmm_map(vmm_addr_space_t *addr_space, void *addr, u64 len, vmm_flags_t flags)
{
    uptr caddr = (uptr)addr;

    if (len == 0 or len % ARCH_PAGE_GRAN !=0)
        return NULL;
    if (caddr % ARCH_PAGE_GRAN != 0)
    {
        if (flags & VMM_FIXED)
            return NULL;
        else
            caddr -= caddr % ARCH_PAGE_GRAN;
    }

    FOREACH (n, addr_space->segments)
    {
           
    }
}

vmm_addr_space_t vmm_new_addr_space()
{
    vmm_addr_space_t addr_space;

    addr_space.slock    = SLOCK_INIT;
    addr_space.segments = LIST_INIT;
    addr_space.ptm_map  = arch_ptm_new_map();

    return addr_space;
}

void vmm_init()
{
    
}
