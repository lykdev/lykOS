#include "proc.h"

#include <core/mm/kmem.h>
#include <core/mm/vmm.h>
#include <core/tasking/sched.h>

#include <utils/string.h>

/// @brief Last ID assigned to a process.
static u64 g_last_id = 0;

proc_t *proc_new(char *name, uint flags)
{
    proc_t *proc = kmem_alloc(sizeof(proc_t));

    proc->id = g_last_id++;
    strcpy(proc->name, name);
    if (flags & PROC_KERNEL)
        proc->addr_space = &vmm_kernel_addr_space;
    proc->threads = LIST_INIT;

    list_append(&g_proc_list, &proc->list_elem);
    return proc;
}

proc_t* proc_find_id(u64 id)
{
    FOREACH (n, g_proc_list)
    {
        proc_t *proc = LIST_GET_CONTAINER(n, proc_t, list_elem);
        if (proc->id == id)
            return proc;
    }
    return NULL;
}
