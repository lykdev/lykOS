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
#include <core/tasking/proc.h>

#include <common/elf.h>

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
    
    proc_t *proc = proc_new("TEST", PROC_USER);
    vfs_node_t *node;
    vfs_lookup("initrd/a.out", &node);
    uptr entry = elf_load_exec(node, proc->addr_space);

    tasking_init();

    log("Kernel end.");
    arch_cpu_halt();
}
