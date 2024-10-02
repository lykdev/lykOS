#include <utils/limine/requests.h>

#include <arch/cpu.h>

#include <core/graphics/video.h>
#include <core/mm/pmm.h>

#include <utils/def.h>
#include <utils/log.h>

u64 a[500000];

void _entry()
{
    video_init();
    log(0, "Kernel start.");
    log(0, "Kernel compiled on %s at %s.", __DATE__, __TIME__);
    
    pmm_init();

    for (size_t i = 0; i < 124000; i++)
        a[i] = (u64) pmm_alloc(0);

    for (size_t i = 0; i < 124000; i++)
        pmm_free((void*)a[i]);    

    pmm_debug_info();

    for (size_t i = 0; i < 1000; i++)
        a[i] = (u64) pmm_alloc(i % PMM_MAX_ORDER);

    for (size_t i = 0; i < 1000; i++)
        pmm_free((void*)a[i]);   

    // pmm_debug_info();

    log(0, "Kernel end.");
    arch_cpu_halt();
}
