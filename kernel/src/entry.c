#include <lib/limine.h>
#include <lib/utils.h>

LIMINE_BASE_REVISION(1)

#include <video/video.h>
#include <acpi/acpi.h>

#if defined (__x86_64__)
    #include <arch/x86_64/tables/gdt.h>
    #include <arch/x86_64/tables/idt.h>
#endif

#include <mm/pmm.h>
#include <mm/vmm.h>

void _start()
{
    video_init();
    log("This build has been compiled on %s at %s.", __DATE__, __TIME__);

    gdt_init();
    idt_init();
    
    pmm_init();
    vmm_init();

    acpi_init();

    halt();
}