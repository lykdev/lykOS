#include "proc.h"

#include <arch/types.h>
#include <common/assert.h>
#include <common/hhdm.h>
#include <common/log.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <mm/vmm.h>
#include <sys/thread.h>

/// @brief Last ID assigned to a process.
static pid_t g_last_id = 0;

list_t g_proc_list = LIST_INIT;

proc_t *proc_new(proc_type_t type)
{
    proc_t *proc = heap_alloc(sizeof(proc_t));

    *proc = (proc_t) {
        .id = g_last_id++,
        .type = type,
        .addr_space = (type == PROC_KERNEL) ? g_vmm_kernel_addr_space : vmm_new_addr_space(ARCH_USER_MIN_VIRT, ARCH_USER_MAX_VIRT),
        .threads = LIST_INIT,
        .resource_table = resource_table_new(),
        .slock = SPINLOCK_INIT,
        .list_node = LIST_NODE_INIT,
        .ref_count = 0
    };

    resource_table_expand(&proc->resource_table, 16);
    //resource_create_at(&proc->resource_table, 1, stdout_new(), 0, RESOURCE_WRITE, false);

    list_append(&g_proc_list, &proc->list_node);
    return proc;
}

proc_t *proc_find_id(pid_t id)
{
    FOREACH(n, g_proc_list)
    {
        proc_t *proc = LIST_GET_CONTAINER(n, proc_t, list_node);
        if (proc->id == id)
            return proc;
    }
    return NULL;
}
