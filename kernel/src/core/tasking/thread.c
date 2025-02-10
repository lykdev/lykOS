#include "tasking.h"

#include <arch/thread.h>

#include <core/mm/pmm.h>
#include <core/mm/kmem.h>

#include <utils/assert.h>
#include <utils/hhdm.h>
#include <utils/string.h>

/// @brief Last ID assigned to a thread.
static u64 g_last_id = 0;
list_t g_thread_list = LIST_INIT;

thread_t *thread_new(proc_t *parent_proc, void *entry)
{
    ASSERT(parent_proc != NULL);

    thread_t *thread = kmem_alloc(sizeof(thread_t));
    *thread = (thread_t) {
#if defined (__x86_64__)
        .self = thread,
#endif
        .id = g_last_id++,
        .parent_proc = parent_proc,
        .assigned_core = NULL
    };

    thread->kernel_stack = (uptr)pmm_alloc(0) + HHDM + ARCH_PAGE_GRAN - sizeof(arch_thread_init_stack_t);
    memset((void*)thread->kernel_stack, 0, sizeof(arch_thread_init_stack_t));
    ((arch_thread_init_stack_t*)thread->kernel_stack)->entry = (void(*)())entry;

    list_append(&parent_proc->threads, &thread->list_elem_inside_proc);

    return thread;
}
