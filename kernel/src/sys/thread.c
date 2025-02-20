#include "tasking.h"

#include <arch/thread.h>

#include <mm/pmm.h>
#include <mm/kmem.h>

#include <utils/assert.h>
#include <utils/hhdm.h>
#include <utils/string.h>
#include <utils/log.h>

/// @brief Last ID assigned to a thread.
static u64 g_last_id = 0;
list_t g_thread_list = LIST_INIT;

extern void x86_64_thread_userspace_init();

thread_t *thread_new(proc_t *parent_proc, uptr entry)
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

    if (parent_proc->type == PROC_KERNEL)
    {
        thread->kernel_stack = (uptr)pmm_alloc(0) + HHDM + ARCH_PAGE_GRAN - sizeof(arch_thread_init_stack_kernel_t);
        memset((void*)thread->kernel_stack, 0, sizeof(arch_thread_init_stack_kernel_t));
        ((arch_thread_init_stack_kernel_t*)thread->kernel_stack)->entry = entry;
    }
    else if (parent_proc->type == PROC_USER)
    {
        thread->kernel_stack = (uptr)pmm_alloc(0) + HHDM + ARCH_PAGE_GRAN - sizeof(arch_thread_init_stack_user_t);
        memset((void*)thread->kernel_stack, 0, sizeof(arch_thread_init_stack_user_t));
        ((arch_thread_init_stack_user_t*)thread->kernel_stack)->userspace_init =  x86_64_thread_userspace_init;
        ((arch_thread_init_stack_user_t*)thread->kernel_stack)->entry = entry;

        u64 user_stack = vmm_find_space(parent_proc->addr_space, 16 * KIB);
        vmm_map_anon(parent_proc->addr_space, user_stack, 16 * KIB, VMM_FULL);
        ((arch_thread_init_stack_user_t*)thread->kernel_stack)->user_stack = user_stack + 15 * KIB; // Leave 1KiB padding.
    }
    
    list_append(&parent_proc->threads, &thread->list_elem_inside_proc);

    return thread;
}
