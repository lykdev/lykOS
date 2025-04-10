#include "exec.h"

#include <common/elf.h>
#include <common/hhdm.h>
#include <common/log.h>
#include <mm/kmem.h>
#include <sys/proc.h>
#include <sys/thread.h>
#include <tasking/sched.h>

proc_t *exec_load(vfs_node_t *file)
{
    Elf64_Ehdr ehdr;
    file->ops->read(file, 0, &ehdr, sizeof(Elf64_Ehdr));

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

    CLEANUP Elf64_Phdr *ph_table = kmem_alloc(ehdr.e_phentsize * ehdr.e_phnum);
    file->ops->read(file, ehdr.e_phoff, ph_table, ehdr.e_phentsize * ehdr.e_phnum);

    for (uint i = 0; i < ehdr.e_phnum; i++)
    {
        Elf64_Phdr *ph = &ph_table[i];

        if (ph->p_type == PT_LOAD && ph->p_memsz != 0)
        {
            vmm_map_anon(proc->addr_space, ph->p_vaddr, ph->p_memsz, VMM_FULL);
            // TODO: the following line is bad. vmm_virt_to_phys should be called for every 0x1000 into memory
            file->ops->read(file, ph->p_offset, (void *)(vmm_virt_to_phys(proc->addr_space, ph->p_vaddr) + HHDM), ph->p_memsz);
        }
    }

    thread_t *thread = thread_new(proc, ehdr.e_entry);
    // Unsafe.
    list_append(&proc->threads, &thread->list_elem_inside_proc);

    return proc;
}