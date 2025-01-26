#include "drv.h"

#include <core/mm/vmm.h>

#include <common/elf.h>

#include <utils/assert.h>
#include <utils/log.h>

bool drv_load_file(vfs_node_t *file)
{
    ASSERT(file->type == VFS_NODE_FILE);
    
    if (!elf_is_compatible(file))
    {
        log("The .drv file is not ELF compatible. Skipping.");
        return false;
    }

    elf_load_rel(file, &vmm_kernel_addr_space);

    return true;
}
