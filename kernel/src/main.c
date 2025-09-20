#include <arch/cpu.h>

#include <common/assert.h>
#include <common/log.h>

#include <dev/acpi/acpi.h>
#include <dev/device.h>
#include <dev/pci.h>

#include <fs/initrd.h>
#include <fs/vfs.h>

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

    // Init IPC code.
    afunix_init();

    bus_t *bus_platform = heap_alloc(sizeof(bus_t));
    bus_init(bus_platform, "platform");
    bus_register(bus_platform);

    pci_init();

    // Load kernel modules.
    {
        ksym_load_symbols();

        vnode_t *module_dir;
        vfs_open("/modules", &module_dir);
        if (module_dir == NULL || module_dir->type != VNODE_DIR)
            panic("Could not find directory `/modules`.");
        u64 idx = 0;
        const char *name;
        while (module_dir->ops->list(module_dir, &idx, &name), name)
        {
            vnode_t *file;
            module_dir->ops->open(module_dir, name, &file);

            module_t *mod = module_load(file);
            mod->install();
        }
    }

    // Load initial executables.
    {
        vnode_t *init_dir;
        vfs_open("/usr/bin", &init_dir);
        if (init_dir == NULL || init_dir->type != VNODE_DIR)
            panic("Could not find directory `/usr/bin`.");
        u64 idx = 0;
        const char *name;
        while (init_dir->ops->list(init_dir, &idx, &name), name)
        {
            vnode_t *file;
            init_dir->ops->open(init_dir, name, &file);

            proc_t *proc = exec_load(file);
            sched_enqueue(LIST_GET_CONTAINER(proc->threads.head, thread_t, list_node_proc));
        }
    }

    reaper_init();
    smp_init();

    log("Kernel end.");
    arch_cpu_halt();
}
