#include <arch/cpu.h>
#include <arch/int.h>
#include <arch/syscall.h>
#include <arch/init.h>

#include <common/assert.h>
#include <common/limine/requests.h>
#include <common/log.h>
#include <fs/initrd.h>
#include <fs/vfs.h>
#include <graphics/video.h>
#include <lib/def.h>
#include <mm/kmem.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <sys/smp.h>
#include <tasking/sched.h>

#include <module.h>
#include <device.h>

void _entry()
{
    video_init();
    log("Kernel start.");
    log("Kernel compiled on %s at %s.", __DATE__, __TIME__);

    arch_int_init();
    arch_cpu_core_init();

    pmm_init();
    kmem_init();
    vmm_init();

    vfs_init();
    initrd_init();

    arch_syscall_init();

    // vfs_node_t *elf_file;
    // vfs_lookup("/initrd/test.elf", &elf_file);
    // elf_object_t *elf_obj = elf_read(elf_file);
    // ASSERT(elf_is_compatible(elf_obj));
    // proc_t *proc = proc_new(PROC_USER);
    // elf_load_exec(elf_obj, proc->addr_space);
    // thread_t *t = thread_new(proc, elf_get_entry(elf_obj));

    
    //sched_queue_add(t);

    mod_init();

    vfs_node_t *file;
    vfs_lookup("/initrd/main.elf", &file);
    module_t *mod = module_load(file);   

    mod->install();

    //smp_init();

    log("Kernel end.");

    arch_cpu_halt();
}