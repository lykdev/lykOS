#include "vmm.h"

#include <arch/ptm.h>

#include <common/assert.h>
#include <common/hhdm.h>
#include <common/limine/requests.h>
#include <common/log.h>
#include <lib/list.h>
#include <mm/kmem.h>
#include <mm/pmm.h>

#define SEG_INTERSECTS(BASE1, LENGTH1, BASE2, LENGTH2) ((BASE1) < ((BASE2) + (LENGTH2)) && (BASE2) < ((BASE1) + (LENGTH1)))

// GLOBAL DATA

vmm_addr_space_t *g_vmm_kernel_addr_space;

static kmem_cache_t *g_segment_cache;

static void insert_seg(vmm_addr_space_t *addr_space, uptr base, u64 len, vmm_seg_type_t type)
{
    // TODO: free the actual segments.

    FOREACH(n, addr_space->segments)
    {
        vmm_seg_t *seg = LIST_GET_CONTAINER(n, vmm_seg_t, list_elem);

        if (SEG_INTERSECTS(base, len + 1, seg->base, seg->len + 1))
        {
            if (seg->type == type)
            {
                uptr end = base + len < seg->base + seg->len ? seg->base + seg->len : base + len;
                base = base < seg->base ? base : seg->base;
                len = end - base;

                list_remove(&addr_space->segments, n);
            } else
            {
                if (base <= seg->base && base + len >= seg->base + seg->len)
                    list_remove(&addr_space->segments, n);
                else if (base <= seg->base && base + len < seg->base + seg->len)
                {
                    seg->len = seg->base + seg->len - base - len;
                    seg->base = base + len;
                } else if (base > seg->base && base + len >= seg->base + seg->len)
                    seg->len = base - seg->base;
                else
                {
                    vmm_seg_t *new_seg = kmem_alloc_from(g_segment_cache);
                    *new_seg = (vmm_seg_t){.base = base + len, .len = seg->base + seg->len - base - len, .type = seg->type, .list_elem = LIST_NODE_INIT, .addr_space = addr_space};
                    list_insert_after(&addr_space->segments, n, &new_seg->list_elem);

                    seg->len = base - seg->base;
                }
            }
        }
    }

    vmm_seg_t *created_seg = kmem_alloc_from(g_segment_cache);
    *created_seg = (vmm_seg_t){.base = base, .len = len, .type = type, .list_elem = LIST_NODE_INIT, .addr_space = addr_space};

    list_node_t *pos = NULL;
    FOREACH(n, addr_space->segments)
    {
        vmm_seg_t *seg = LIST_GET_CONTAINER(n, vmm_seg_t, list_elem);

        if (seg->base < base)
            pos = n;
    }
    list_insert_after(&addr_space->segments, pos, &created_seg->list_elem);
}

// Actual VMM logic

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

uptr vmm_map_anon(vmm_addr_space_t *addr_space, uptr virt, u64 len, vmm_prot_t prot)
{
    ASSERT(virt % ARCH_PAGE_GRAN == 0 and len % ARCH_PAGE_GRAN == 0);
    slock_acquire(&addr_space->slock);

    insert_seg(addr_space, virt, len, VMM_ANON);
    for (uptr addr = 0; addr < len; addr += ARCH_PAGE_SIZE_4K)
        arch_ptm_map(&addr_space->ptm_map, virt + addr, (uptr)pmm_alloc(0), ARCH_PAGE_SIZE_4K);

    slock_release(&addr_space->slock);
    return virt;
}

uptr vmm_map_direct(vmm_addr_space_t *addr_space, uptr virt, u64 len, vmm_prot_t prot, uptr phys)
{
    ASSERT(virt % ARCH_PAGE_GRAN == 0 and len % ARCH_PAGE_GRAN == 0);
    slock_acquire(&addr_space->slock);

    insert_seg(addr_space, virt, len, VMM_DIRECT);
    for (uptr addr = 0; addr < len; addr += ARCH_PAGE_SIZE_4K)
        arch_ptm_map(&addr_space->ptm_map, virt + addr, phys + addr, ARCH_PAGE_SIZE_4K);

    slock_release(&addr_space->slock);
    return virt;
}

uptr vmm_virt_to_phys(vmm_addr_space_t *addr_space, uptr virt) { return arch_ptm_virt_to_phys(&addr_space->ptm_map, virt); }

vmm_addr_space_t *vmm_new_addr_space(uptr limit_low, uptr limit_high)
{
    vmm_addr_space_t *addr_space = kmem_alloc(sizeof(vmm_addr_space_t));

    *addr_space = (vmm_addr_space_t){.slock = SLOCK_INIT, .segments = LIST_INIT, .limit_low = limit_low, .limit_high = limit_high, .ptm_map = arch_ptm_new_map()};
    return addr_space;
}

void vmm_load_addr_space(vmm_addr_space_t *addr_space) { arch_ptm_load_map(&addr_space->ptm_map); }

void vmm_init()
{
    arch_ptm_init();

    g_vmm_kernel_addr_space = vmm_new_addr_space(ARCH_HIGHER_HALF_START, ARCH_MAX_VIRT_ADDR);

    g_segment_cache = kmem_new_cache("VMM Segment Cache", sizeof(vmm_seg_t));

    vmm_map_direct(g_vmm_kernel_addr_space, HHDM, 4 * GIB, VMM_FULL, 0);
    vmm_map_direct(g_vmm_kernel_addr_space, request_kernel_addr.response->virtual_base, 2 * GIB, VMM_FULL, request_kernel_addr.response->physical_base);

    vmm_load_addr_space(g_vmm_kernel_addr_space);

    log("VMM initialized.");
}
