#include <arch/cpu.h>
#include <arch/int.h>
#include <arch/syscall.h>
#include <arch/init.h>

#include <common/assert.h>
#include <common/limine/requests.h>
#include <common/log.h>
#include <fs/devfs.h>
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

#include <lib/string.h>

extern void dev_fb_init();

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
    devfs_init();

    // Load kernel modules.

    ksym_load_symbols();

    vfs_node_t *module_dir = vfs_lookup("/initrd/modules");
    if (module_dir == NULL || module_dir->type != VFS_NODE_DIR)
        panic("Could not find directory `/initrd/modules`.");
    uint idx = 0;
    const char *name;
    while ((name = module_dir->ops->list(module_dir, &idx)))
    {
        log("Loading module `%s`.", name);
        if (name[0] != '\0')
        {
            vfs_node_t *file = module_dir->ops->lookup(module_dir, name);

            module_t *mod = module_load(file);
            mod->install();
        }
    }

    dev_fb_init();

    u32 pix[100];
    for (size_t i = 0; i < 50; i++)
        pix[i] = 0xFF0000;
    for (size_t i = 50; i < 100; i++)
        pix[i] = 0x00FF00;

    vfs_node_t *fb = vfs_lookup("/dev/fb");
    ASSERT(fb != NULL);
    fb->ops->write(fb, 0, pix, 100 * 4);


    //smp_init();

    log("Kernel end.");

    arch_cpu_halt();
}
