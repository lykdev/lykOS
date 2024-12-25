#include "tasking.h"

#include <arch/cpu.h>

#include <core/mm/pmm.h>
#include <core/mm/kmem.h>
#include <core/tasking/def.h>
#include <core/tasking/proc.h>
#include <core/tasking/thread.h>
#include <core/tasking/sched.h>

#include <utils/log.h>
#include <utils/panic.h>
#include <utils/limine/requests.h>

list_t g_cpu_core_list = LIST_INIT;
list_t g_thread_list = LIST_INIT;
list_t g_proc_list = LIST_INIT;

bool g_smp_initialized = false;

static void thread_idle_func()
{
    while (true)
        sched_yield();    
}

static void core_init(struct limine_smp_info *smp_info)
{
    static slock_t slock = SLOCK_INIT;
    slock_acquire(&slock); // Assure CPU cores are initialized sequentially.

    log("NEW CORE %u", smp_info->extra_argument);

    thread_t *idle_thread = thread_new(proc_find_id(0), &thread_idle_func);
    cpu_core_t *cpu_core = kmem_alloc(sizeof(cpu_core_t));
    *cpu_core = (cpu_core_t) {
        .id = smp_info->extra_argument,
        .idle_thread = idle_thread
    };
    list_append(&g_cpu_core_list, &cpu_core->list_elem);

    idle_thread->assigned_core = cpu_core;
    arch_cpu_write_thread_reg(idle_thread);

    slock_release(&slock);
    arch_cpu_halt();
}

void tasking_init()
{
    if (request_smp.response == NULL)
        panic("Invalid SMP info provided by the bootloader");

    proc_t *idle_proc = proc_new("System Idle Process", true);
    
    for (size_t i = 0; i < request_smp.response->cpu_count; i++)
    {
        struct limine_smp_info *smp_info = request_smp.response->cpus[i];
        smp_info->extra_argument = i;
        __atomic_store_n(&smp_info->goto_address, (limine_goto_address)&core_init, __ATOMIC_SEQ_CST);
    }
}