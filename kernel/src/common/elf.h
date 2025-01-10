#pragma once

#include <core/fs/vfs.h>
#include <core/mm/vmm.h>

bool elf_load(vfs_node_t *file, vmm_addr_space_t *addr_space);
