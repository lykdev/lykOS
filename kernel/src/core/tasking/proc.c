#include "tasking.h"

#include <core/mm/kmem.h>
#include <core/mm/vmm.h>

#include <utils/assert.h>
#include <utils/hhdm.h>
#include <utils/string.h>

/// @brief Last ID assigned to a process.
static u64 g_last_id = 0;
list_t g_proc_list = LIST_INIT;

proc_t *proc_new()
{
    proc_t *proc = kmem_alloc(sizeof(proc_t));

    proc->id = g_last_id++;
    proc->addr_space = vmm_new_addr_space(0, HHDM - 1);
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
