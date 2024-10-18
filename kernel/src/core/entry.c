#include <utils/limine/requests.h>

#include <arch/cpu.h>

#include <core/fs/vfs.h>
#include <core/fs/initrd.h>
#include <core/graphics/video.h>
#include <core/mm/pmm.h>
#include <core/mm/vmm.h>

#include <utils/def.h>
#include <utils/log.h>

void _entry()
{
    video_init();
    log("Kernel start.");
    log("Kernel compiled on %s at %s.", __DATE__, __TIME__);
    
    pmm_init();
    vmm_init();

    initrd_init();

    log("Kernel end.");
    arch_cpu_halt();
}
