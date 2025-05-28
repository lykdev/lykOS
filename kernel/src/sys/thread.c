#include "thread.h"

#include <arch/thread.h>
#include <arch/types.h>
#include <arch/x86_64/fpu.h>

#include <common/assert.h>
#include <common/hhdm.h>
#include <common/log.h>
#include <lib/math.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <mm/pmm.h>
#include <sys/smp.h>
#include <tasking/sched.h>

/// @brief Last ID assigned to a thread.
static u64 g_last_id = 0;

thread_t *thread_new(proc_t *parent_proc, uptr entry)
{
    ASSERT(parent_proc != NULL);

    thread_t *thread = heap_alloc(sizeof(thread_t));
    *thread = (thread_t) {
        .id = g_last_id++,
        .parent_proc = parent_proc,
        .assigned_core = NULL,
        .list_elem_thread = LIST_NODE_INIT,
        .list_elem_inside_proc = LIST_NODE_INIT,
        .mutex_wait_queue_node = LIST_NODE_INIT
    };
    arch_thread_context_init(&thread->context, thread->parent_proc, parent_proc->type == PROC_USER ? true : false, entry);

    list_append(&parent_proc->threads, &thread->list_elem_inside_proc);

    return thread;
}
