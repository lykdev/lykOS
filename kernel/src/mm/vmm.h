#pragma once

#include <arch/ptm.h>
#include <arch/types.h>
#include <common/sync/spinlock.h>
#include <fs/vfs.h>
#include <lib/def.h>
#include <lib/list.h>

#define VMM_PROT_NONE  0x0
#define VMM_PROT_READ  0x1
#define VMM_PROT_WRITE 0x2
#define VMM_PROT_EXEC  0x4
#define VMM_PROT_FULL  (VMM_PROT_READ | VMM_PROT_WRITE | VMM_PROT_EXEC)

#define VMM_MAP_PRIVATE         0x01
#define VMM_MAP_SHARED          0x02
#define VMM_MAP_ANON            0x04
#define VMM_MAP_FIXED           0x08
#define VMM_MAP_FIXED_NOREPLACE 0x10
#define VMM_MAP_POPULATE        0x20

#define VMM_MAP_FAILED          ((void *)(-1))

typedef struct
{
    spinlock_t slock;
    list_t segments;
    uptr limit_low;
    uptr limit_high;

    arch_ptm_map_t ptm_map;
}
vmm_addr_space_t;

extern vmm_addr_space_t *g_vmm_kernel_addr_space;

bool vmm_pagefault_handler(vmm_addr_space_t *as, uptr addr);

void *vmm_map_vnode(vmm_addr_space_t *as, uptr virt, u64 len, int prot, int flags, vnode_t *vnode, u64 offset);

void vmm_map_kernel(vmm_addr_space_t *as, uptr virt, u64 len, int prot, uptr phys);

uptr vmm_virt_to_phys(vmm_addr_space_t *as, uptr virt);

vmm_addr_space_t *vmm_new_addr_space(uptr limit_low, uptr limit_high);

void vmm_load_addr_space(vmm_addr_space_t *as);

u64 vmm_copy_to(vmm_addr_space_t *dest_as, uptr dest_addr, void *src, u64 count);

u64 vmm_zero_out(vmm_addr_space_t *dest_as, uptr dest_addr, u64 count);

void vmm_init();
