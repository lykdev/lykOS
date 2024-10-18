#pragma once

#include <arch/ptm.h>

#include <utils/def.h>
#include <utils/list.h>
#include <utils/slock.h>

typedef struct
{
    slock_t slock;
    list_t segments;
    arch_ptm_map_t ptm_map;
}
vmm_addr_space_t;

typedef enum
{
    VMM_FIXED,
}
vmm_flags_t;

typedef enum
{
    VMM_ANON,
}
vmm_seg_type_t;

typedef struct
{
    vmm_addr_space_t *addr_space;
    vmm_seg_type_t type;
    uptr base;
    u64  len;

    list_node_t list_elem;
}
vmm_seg_t;

void* vmm_map(vmm_addr_space_t *addr_space, void *addr, u64 len, vmm_flags_t flags);

vmm_addr_space_t vmm_new_addr_space();

void vmm_init();
