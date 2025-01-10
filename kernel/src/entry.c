#include <utils/limine/requests.h>

#include <arch/cpu.h>
#include <arch/int.h>

#include <core/fs/vfs.h>
#include <core/fs/initrd.h>
#include <core/graphics/video.h>
#include <core/mm/kmem.h>
#include <core/mm/pmm.h>
#include <core/mm/vmm.h>
#include <core/tasking/tasking.h>

#include <utils/def.h>
#include <utils/log.h>

static void load_drivers()
{
    vfs_node_t *dir;
    vfs_lookup("/initrd/", &dir);

    uint i = 0;
    while (true)
    {
        char *file_name;
        dir->ops->list(dir, &i, &file_name);

        if (file_name == NULL)
            break;

        log("%s", file_name);
    }
}

void *a[5000];

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

    //load_drivers();

    //tasking_init();

    log("Kernel end.");
    arch_cpu_halt();
}
