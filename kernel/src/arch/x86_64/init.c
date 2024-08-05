#include <limine.h>

#include <arch/cpu.h>
#include <arch/x86_64/tables/gdt.h>
#include <arch/x86_64/tables/idt.h>
#include <arch/x86_64/pit.h>
#include <arch/x86_64/lapic.h>

#include <core/graphics/video.h>
#include <core/mm/pmm.h>
#include <core/mm/vmm.h>
#include <core/tasking/sched.h>
#include <core/vfs.h>

#include <dev/acpi/acpi.h>
#include <dev/pci.h>

#include <lib/def.h>
#include <lib/log.h>
#include <lib/assert.h>

__attribute__((used))
static volatile LIMINE_BASE_REVISION(1);

void x86_64_entry()
{
    log("KERNEL START");

    // Ensure the bootloader actually understands our base revision.
    ASSERT(LIMINE_BASE_REVISION_SUPPORTED != false);

    // Video

    video_init();
    
    // Tables

    x86_64_gdt_load();
    x86_64_idt_load();

    // APIC

    x86_64_lapic_timer_init();

    // Memory

    pmm_init();
    vmm_init();

    // Tasking

    sched_init();

    // Other

    vfs_init();

    // Devices

    acpi_init();
    pci_init();

    log("KERNEL END");
    cpu_lcore_halt();
}
