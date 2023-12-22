#include <lib/limine.h>
#include <lib/utils.h>

LIMINE_BASE_REVISION(0)

#include <video/video.h>
#include <acpi/acpi.h>

#if defined (__x86_64__)
    #include <arch/x86_64/cpu.h>
#endif

#include <mm/pmm.h>
#include <mm/vmm.h>

#include <proc/smp.h>
#include <proc/sched.h>

void _start()
{
    video_init();
    log("This build has been compiled on %s at %s.", __DATE__, __TIME__);

    cpu_core_setup();

    pmm_init();
    vmm_init();

    acpi_init();    

    // smp_init();

    sched_init();

    halt();
}