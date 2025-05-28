#include <lib/list.h>
#include <sys/proc.h>
#include <arch/cpu.h>
#include <arch/int.h>
#include <arch/syscall.h>
#include <arch/init.h>
#include <common/assert.h>
#include <common/log.h>
#include <dev/acpi/acpi.h>
#include <dev/pci.h>
#include <fs/initrd.h>
#include <fs/vfs.h>
#include <fs/sysfs.h>
#include <graphics/video.h>
#include <lib/def.h>
#include <mm/heap.h>
#include <mm/kmem.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <sys/exec.h>
#include <sys/ksym.h>
#include <sys/module.h>
#include <sys/thread.h>
#include <sys/smp.h>
#include <tasking/sched.h>

#include <lib/string.h>

#include <arch/x86_64/syscall.h>

extern void dev_fb_init();

void _entry()
{
    video_init();
    log("Kernel start.");
    log("Kernel compiled on %s at %s.", __DATE__, __TIME__);

    acpi_init();

    pmm_init();
    kmem_init();
    heap_init();
    vmm_init();

    arch_init();
    arch_cpu_core_init(0);

    vfs_init();
    initrd_init();

    vfs_mount("/sys", sysfs_new_mp("sys"));
    vfs_mount("/dev", sysfs_new_mp("dev"));

    // List PCI devices. Required for loading device drivers.
    pci_list();

    // Load kernel modules.
    {
        ksym_load_symbols();

        vfs_node_t *module_dir = vfs_lookup("/modules");
        if (module_dir == NULL || module_dir->type != VFS_NODE_DIR)
            panic("Could not find directory `/modules`.");
        u64 idx = 0;
        const char *name;
        while ((name = module_dir->dir_ops->list(module_dir, &idx)))
        {
            vfs_node_t *file = module_dir->dir_ops->lookup(module_dir, name);
            module_t *mod = module_load(file);

            if (mod->probe())
                mod->install();
        }
    }

    dev_fb_init();

    // Load initial executables.
    {
        vfs_node_t *init_dir = vfs_lookup("/usr/bin");
        if (init_dir == NULL || init_dir->type != VFS_NODE_DIR)
            panic("Could not find directory `/usr/bin`.");
        u64 idx = 0;
        const char *name;
        while ((name = init_dir->dir_ops->list(init_dir, &idx)))
        {
            vfs_node_t *file = init_dir->dir_ops->lookup(init_dir, name);
            proc_t *proc = exec_load(file);
            sched_queue_add(LIST_GET_CONTAINER(proc->threads.head, thread_t, list_elem_inside_proc));
        }
    }

    smp_init();

    log("Kernel end.");
    while (true)
        arch_cpu_halt();
}
