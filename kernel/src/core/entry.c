#include <utils/limine/requests.h>

#include <arch/cpu.h>

#include <core/graphics/video.h>
#include <core/mm/pmm.h>

#include <utils/def.h>
#include <utils/log.h>

void _entry()
{
    video_init();
    log(0, "Kernel start.");
    log(0, "Kernel compiled on %s at %s.", __DATE__, __TIME__);
    
    pmm_init();
    

    log(0, "Kernel end.");
    arch_cpu_halt();
}
