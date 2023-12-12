#include <lib/limine.h>
#include <lib/utils.h>

LIMINE_BASE_REVISION(1)

#if defined (__x86_64__)
        #include <core/arch/x86_64/init.h>
#elif defined (__aarch64__)
        #include <core/arch/aarch64/init.h>
#endif

#include <hal/video/video.h>
#include <hal/acpi/acpi.h>

#include <core/pmm.h>
#include <core/vmm.h>

void _start()
{
    video_init();
    pmm_init();
    vmm_init();
    acpi_init();

    log("This build has been compiled on %s at %s.", __DATE__, __TIME__);
    
    #if defined (__x86_64__)
        x86_64_init();
    #elif defined (__aarch64__)
        aarch64_init();
    #endif

    // We're done, just hang...
    halt();
}