#include "thread.h"

#include <core/mm/pmm.h>
#include <core/mm/kmem.h>
#include <core/tasking/sched.h>

/// @brief Last ID assigned to a thread.
static u64 g_last_id = 0;

thread_t *thread_new(proc_t *parent_proc, void *entry)
{
    thread_t *thread = kmem_alloc(sizeof(thread_t));
    *thread = (thread_t) {
#if defined (__x86_64__)
        .self = thread,
#endif
        .id = g_last_id++,
        .parent_proc = parent_proc,
        .assigned_core = NULL,
        .stack = pmm_alloc(0)
    };
    list_append(&g_proc_list, &thread->list_elem_thread);
    
    return thread;
}
