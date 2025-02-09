#pragma once

#include <core/fs/vfs.h>
#include <core/mm/vmm.h>

bool elf_is_compatible(vfs_node_t *file);

bool elf_load_rel(vfs_node_t *file, vmm_addr_space_t *addr_space);

uptr elf_load_exec(vfs_node_t *file, vmm_addr_space_t *addr_space);
