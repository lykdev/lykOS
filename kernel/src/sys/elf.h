#pragma once

#include <fs/vfs.h>
#include <mm/vmm.h>

bool elf_load_relocatable(vfs_node_t *file);