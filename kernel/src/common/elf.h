#pragma once

#include <core/fs/vfs.h>
#include <core/mm/vmm.h>

typedef struct elf_object elf_object_t;

elf_object_t *elf_read(vfs_node_t *file);

bool elf_is_compatible(elf_object_t *elf_obj);

void elf_load_exec(elf_object_t *elf_obj, vmm_addr_space_t *as);

uptr elf_get_entry(elf_object_t *elf_obj);
 