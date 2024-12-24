#include "sched.h"

#include <arch/cpu.h>

#include <core/mm/heap.h>

#include <utils/limine/requests.h>
#include <utils/log.h>
#include <utils/panic.h>
#include <utils/slock.h>

extern __attribute__((naked)) void arch_sched_context_switch(thread_t *curr, thread_t *next);

list_t sched_cpu_core_list = LIST_INIT;
list_t sched_thread_list = LIST_INIT;
list_t sched_proc_list = LIST_INIT;

static slock_t slock = SLOCK_INIT;

static thread_t* sched_next()
{
    thread_t *ret = NULL; 

    slock_acquire(&slock);

    list_node_t *node = list_pop_head(&sched_thread_list);
    if (node != NULL)
        ret = LIST_GET_CONTAINER(node, thread_t, list_elem_thread);

    slock_release(&slock);

    return ret;
}

static void sched_queue_add(thread_t *thread)
{
    slock_acquire(&slock);

    list_append(&sched_thread_list, &thread->list_elem_thread);

    slock_release(&slock);
}

void sched_yield()
{
    thread_t *curr = arch_cpu_read_thread_reg();
    thread_t *next = sched_next();
    if (next == NULL)
        next = ((thread_t*)arch_cpu_read_thread_reg())->assigned_core->idle_thread;

    next->assigned_core = curr->assigned_core;
    curr->assigned_core = NULL;
    arch_cpu_write_thread_reg(next);

    arch_sched_context_switch(curr, next);

    sched_queue_add(curr);
}

static void thread_idle_func()
{
    while (true)
        sched_yield();    
}

static void core_init(struct limine_smp_info *smp_info)
{
    static slock_t slock = SLOCK_INIT;
    slock_acquire(&slock); // Assure CPU cores are initialized sequentially.

    log("NEW CORE");

    thread_t *idle_thread = thread_new(proc_find_id(0), &thread_idle_func);

    cpu_core_t *cpu_core = heap_alloc(sizeof(cpu_core_t));
    *cpu_core = (cpu_core_t) {
        .id = smp_info->extra_argument,
        .idle_thread = idle_thread
    };
    list_append(&sched_cpu_core_list, &cpu_core->list_elem);

    slock_release(&slock);
    arch_cpu_halt();
}

void sched_init()
{
    if (request_smp.response == NULL)
        panic("Invalid SMP info provided by the bootloader");
    log("Starting up additional CPU cores");

    proc_t *idle_proc = proc_new("System Idle Process", true);
    
    for (size_t i = 0; i < request_smp.response->cpu_count; i++)
    {
        struct limine_smp_info *smp_info = request_smp.response->cpus[i];
        smp_info->extra_argument = i;
        __atomic_store_n(&smp_info->goto_address, (limine_goto_address)&core_init, __ATOMIC_SEQ_CST);
    }
}
