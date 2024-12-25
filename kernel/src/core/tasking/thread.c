#include "thread.h"

#include <core/mm/heap.h>
#include <core/tasking/sched.h>

/// @brief Last ID assigned to a thread.
static u64 g_last_id = 0;

thread_t *thread_new(proc_t *parent_proc, void *entry)
{
    thread_t *thread = heap_alloc(sizeof(thread_t));

    thread->id = g_last_id++;

    list_append(&parent_proc->threads, &thread->list_elem_inside_proc);
    list_append(&g_proc_list, &thread->list_elem_thread);
    
    return thread;
}
