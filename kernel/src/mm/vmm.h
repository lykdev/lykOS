#pragma once

#include <lib/slock.h>
#include <lib/utils.h>

#define VMM_PRESENT 1
#define VMM_WRITE 1 << 1
#define VMM_USER 1 << 2
#define VMM_NOEXE 1 << 3

struct vmm_pagemap
{
    u64 *top_level;
    slock lock;
};

extern struct vmm_pagemap kernelmap;

void vmm_init();
void vmm_new_pagemap(struct vmm_pagemap *map);
void vmm_map_page(struct vmm_pagemap *map, uptr virt, uptr phys, u64 flags);
void vmm_switch_to_map(struct vmm_pagemap *map);
