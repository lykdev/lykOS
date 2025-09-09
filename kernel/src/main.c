#include <arch/cpu.h>

#include <common/assert.h>
#include <common/log.h>

#include <dev/acpi/acpi.h>
#include <dev/devman.h>
#include <dev/pci.h>

#include <fs/initrd.h>
#include <fs/vfs.h>
#include <fs/sysfs.h>

#include <graphics/video.h>

#include <lib/def.h>
#include <lib/list.h>
#include <lib/string.h>

#include <mm/heap.h>
#include <mm/kmem.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

#include <sys/exec.h>
#include <sys/ksym.h>
#include <sys/module.h>
#include <sys/proc.h>
#include <sys/reaper.h>
#include <sys/sched.h>
#include <sys/smp.h>
#include <sys/thread.h>

extern void dev_fb_init();

extern void afunix_init();

void kernel_main()
{
    log("Kernel main");

    pmm_init();
    kmem_init();
    heap_init();
    vmm_init();

    vfs_init();
    initrd_init();

    vfs_mount("/sys", sysfs_new_mp("sys"));

    // Init IPC code.
    afunix_init();

    devman_init();

    // Load kernel modules.
    {
        ksym_load_symbols();

        vnode_t *module_dir;
        vfs_open("/modules", &module_dir);
        if (module_dir == NULL || module_dir->type != VFS_NODE_DIR)
            panic("Could not find directory `/modules`.");
        u64 idx = 0;
        const char *name;
        while (module_dir->ops->list(module_dir, &idx, &name), name)
        {
            vnode_t *file;
            module_dir->ops->lookup(module_dir, name, &file);

            module_t *mod = module_load(file);
            if (mod->probe())
                mod->install();
        }
    }

    dev_fb_init();

    // Load initial executables.
    {
        vnode_t *init_dir;
        vfs_open("/usr/bin", &init_dir);
        if (init_dir == NULL || init_dir->type != VFS_NODE_DIR)
            panic("Could not find directory `/usr/bin`.");
        u64 idx = 0;
        const char *name;
        while (init_dir->ops->list(init_dir, &idx, &name), name)
        {
            vnode_t *file;
            init_dir->ops->lookup(init_dir, name, &file);

            proc_t *proc = exec_load(file);
            sched_enqueue(LIST_GET_CONTAINER(proc->threads.head, thread_t, list_node_proc));
        }
    }

    reaper_init();
    smp_init();

    log("Kernel end.");
    arch_cpu_halt();
}
