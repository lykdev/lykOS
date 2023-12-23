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

    void *a = pmm_alloc();
    void *b = pmm_alloc();
    void *c = pmm_alloc();
    void *d = pmm_alloc();
    void *e = pmm_alloc();

    log("%llx", a);
    log("%llx", b);
    log("%llx", c);
    log("%llx", d);
    log("%llx", e);

    vmm_init();

    acpi_init();    

    // smp_init();

    sched_init();

    halt();
}