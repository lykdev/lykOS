#include "exec.h"

#include <arch/types.h>
#include <common/assert.h>
#include <common/elf.h>
#include <common/hhdm.h>
#include <common/log.h>
#include <lib/math.h>
#include <mm/heap.h>
#include <mm/pmm.h>
#include <sys/proc.h>
#include <sys/thread.h>
#include <tasking/sched.h>

proc_t *exec_load(vnode_t *file)
{
    log("Loading executable `%s`.", file->name);

    // Variable to be used as output parameter for file read/write operations.
    u64 count;

    Elf64_Ehdr ehdr;
    if (file->ops->read(file, 0, &ehdr, sizeof(Elf64_Ehdr), &count) < 0
    ||  count != sizeof(Elf64_Ehdr))
    {
        log("Could not read file header!");
        return NULL;
    }

    if (!elf_check_compatibility(&ehdr))
    {
        log("Incompatible ELF file `%s`!", file->name);
        return NULL;
    }
    if (ehdr.e_type != ET_EXEC)
    {
        log("Provided ELF file `%s` isn't a executable object file!", file->name);
        return NULL;
    }

    proc_t *proc = proc_new(PROC_USER);

    CLEANUP Elf64_Phdr *ph_table = heap_alloc(ehdr.e_phentsize * ehdr.e_phnum);
    file->ops->read(file, ehdr.e_phoff, ph_table, ehdr.e_phentsize * ehdr.e_phnum, &count);

    for (uint i = 0; i < ehdr.e_phnum; i++)
    {
        Elf64_Phdr *ph = &ph_table[i];

        if (ph->p_type == PT_LOAD && ph->p_memsz != 0)
        {
            uptr start = FLOOR(ph->p_vaddr, ARCH_PAGE_GRAN);
            uptr end   = CEIL(ph->p_vaddr + ph->p_memsz, ARCH_PAGE_GRAN);
            u64  diff  = end - start;

            vmm_map_vnode(
                proc->addr_space,
                start,
                diff,
                VMM_PROT_FULL,
                VMM_MAP_ANON | VMM_MAP_POPULATE | VMM_MAP_FIXED | VMM_MAP_PRIVATE,
                NULL,
                0
            );

            // TODO: stop using pmm
            ASSERT(pmm_order_to_pagecount(10) * ARCH_PAGE_GRAN >= ph->p_filesz);
            void *buf = pmm_alloc(10);
            file->ops->read(file, ph->p_offset, (void*)((uptr)buf + HHDM), ph->p_filesz, &count);
            vmm_copy_to(proc->addr_space, ph->p_vaddr, (void*)((uptr)buf + HHDM), ph->p_filesz);
            pmm_free(buf);
        }
    }

    thread_new(proc, ehdr.e_entry);

    return proc;
}
