#include <utils/limine/requests.h>

#include <arch/cpu.h>
#include <arch/int.h>
#include <arch/syscall.h>

#include <core/fs/vfs.h>
#include <core/fs/initrd.h>
#include <core/graphics/video.h>
#include <core/mm/kmem.h>
#include <core/mm/pmm.h>
#include <core/mm/vmm.h>
#include <core/tasking/tasking.h>
#include <core/tasking/sched.h>
#include <core/tasking/smp.h>

#include <common/elf.h>

#include <utils/def.h>
#include <utils/log.h>

void f1()
{
    for (int i = 0; i < 10; i++)
    {
        log("A-%i", i);
        sched_yield();
    }
        
    while (true)
        sched_yield();
}

void f2()
{
    for (int i = 0; i < 5; i++)
    {
        log("B-%i", i);
        sched_yield();
    }
        
    while (true)
        sched_yield();
}

void _entry()
{
    video_init();
    log("Kernel start.");
    log("Kernel compiled on %s at %s.", __DATE__, __TIME__);

    arch_int_init();
    
    pmm_init();
    kmem_init();
    vmm_init();

    vfs_init();
    initrd_init();

    arch_syscall_init();
    
    proc_t *proc = proc_new();
    thread_t *t1 = thread_new(proc, f1);
    sched_queue_add(t1);
    thread_t *t2 = thread_new(proc, f2);
    sched_queue_add(t2);

    smp_init();

    log("Kernel end.");
    arch_cpu_halt();
}
