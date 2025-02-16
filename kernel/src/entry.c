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

#include <utils/assert.h>
#include <utils/def.h>
#include <utils/log.h>

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
    
    vfs_node_t *elf_file;
    vfs_lookup("/initrd/main.elf", &elf_file);

    elf_object_t *elf_obj = elf_read(elf_file);
    ASSERT(elf_is_compatible(elf_obj));

    proc_t *proc = proc_new(PROC_USER);
    elf_load_exec(elf_obj, proc->addr_space);
    thread_t *t = thread_new(proc, elf_get_entry(elf_obj));
    sched_queue_add(t);

    smp_init();

    log("Kernel end.");
    arch_cpu_halt();
}
