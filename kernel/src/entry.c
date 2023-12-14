#include <lib/limine.h>
#include <lib/utils.h>

LIMINE_BASE_REVISION(1)

#include <video/video.h>
#include <acpi/acpi.h>

#include <int/idt.h>

#include <mm/pmm.h>
#include <mm/vmm.h>

void _start()
{
    video_init();
    log("This build has been compiled on %s at %s.", __DATE__, __TIME__);

    idt_init();
    
    pmm_init();
    vmm_init();

    acpi_init();

    // We're done, just hang...
    halt();
}