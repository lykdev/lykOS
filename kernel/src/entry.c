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
#include <sys/ksym.h>
#include <sys/module.h>
#include <sys/smp.h>
#include <tasking/sched.h>

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

    // Load kernel modules.

    ksym_load_symbols();
    vfs_node_t *module_dir;
    vfs_lookup("/initrd/modules", &module_dir);
    if (module_dir == NULL || module_dir->type != VFS_NODE_DIR)
        panic("Could not find directory `/initrd/modules`.");
    int idx = 0;
    const char *name;
    while (module_dir->ops->list(module_dir, &idx, &name))
    {
        log("Loading module `%s`.", name);
        if (name[0] != '\0')
        {
            vfs_node_t *file;
            module_dir->ops->lookup(module_dir, name, &file);

            module_t *mod = module_load(file);
            mod->install();
        }    
    }
    
    smp_init();

    log("Kernel end.");

    arch_cpu_halt();
}