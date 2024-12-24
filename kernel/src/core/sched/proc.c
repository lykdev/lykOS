#include "sched.h"

#include <core/mm/heap.h>
#include <core/mm/vmm.h>

#include <utils/string.h>

/// @brief Last ID assigned to a process.
static u64 g_last_id = 0;

proc_t *proc_new(char *name, bool privileged)
{
    proc_t *proc = heap_alloc(sizeof(proc_t));

    proc->id = g_last_id++;
    strcpy(proc->name, name);
    if (!privileged)
        proc->addr_space = vmm_new_addr_space(0, ARCH_HIGHER_HALF_START - 1);
    else
        proc->addr_space = NULL;
    proc->threads = LIST_INIT;

    list_append(&sched_proc_list, &proc->list_elem);
    return proc;
}

proc_t* proc_find_id(u64 id)
{
    FOREACH (n, sched_proc_list)
    {
        proc_t *proc = LIST_GET_CONTAINER(n, proc_t, list_elem);
        if (proc->id == id)
            return proc;
    }
    return NULL;
}
