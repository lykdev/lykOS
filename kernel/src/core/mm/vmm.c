#include "vmm.h"

#include <core/mm/pmm.h>
#include <utils/hhdm.h>
#include <utils/list.h>
#include <utils/log.h>

// #define ADDR_IN_BOUNDS(ADDRESS_SPACE, ADDRESS) ((ADDRESS) >= (ADDRESS_SPACE)->start && (ADDRESS) < (ADDRESS_SPACE)->end)
#define ADDR_IN_SEGMENT(ADDRESS, BASE, LENGTH) ((ADDRESS) >= (BASE) && (ADDRESS) < ((BASE) + (LENGTH)))

// #define SEG_IN_BOUNDS(ADDRESS_SPACE, BASE, LENGTH) (ADDR_IN_BOUNDS((ADDRESS_SPACE), (BASE)) && ((ADDRESS_SPACE)->end - (BASE)) >= (LENGTH))
#define SEG_INTERSECTS(BASE1, LENGTH1, BASE2, LENGTH2) ((BASE1) < ((BASE2) + (LENGTH2)) && (BASE2) < ((BASE1) + (LENGTH1)))

list_t free_seg_list   = LIST_INIT;

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

void vmm_map_seg(vmm_addr_space_t *addr_space, uptr base, uptr len, vmm_seg_type_t type)
{
    FOREACH (n, addr_space->segments)
    {
        vmm_seg_t *seg = LIST_GET_CONTAINER(n, vmm_seg_t, list_elem);
        
        if (SEG_INTERSECTS(base, len, seg->base, seg->len))
        {
            if (base <= seg->base && base + len >= seg->base + seg->len)
            {
                n = n->next;
                list_remove(&addr_space->segments, n->prev);             
            }
            else if (base <= seg->base && base + len < seg->base + seg->len)
            {
                seg->len = seg->base + seg->len - base - len;
                seg->base = base + len;
            }   
            else if (base > seg->base && base + len >= seg->base + seg->len)
            {
                seg->len = base - seg->base;
            }                
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

                n = n->next;
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
    vmm_addr_space_t a = vmm_new_addr_space();

    vmm_map_seg(&a, 0x1000, 0x5000, VMM_ANON);
    vmm_map_seg(&a, 0x2000, 0x6000, VMM_ANON);
    vmm_map_seg(&a, 0x3000, 0x1000, VMM_ANON);

    vmm_map_seg(&a, 0x1500, 0x2000, VMM_ANON);
    vmm_map_seg(&a, 0x1750, 0x8000, VMM_ANON);

    log(2, "A");

    FOREACH (n, a.segments)
    {
        vmm_seg_t *seg = LIST_GET_CONTAINER(n, vmm_seg_t, list_elem);

        log(2, "%#llx, %#llx, %d", seg->base, seg->len, seg->type);
    }

    log(2, "B");
}
