#include <limine.h>

#include <arch/cpu.h>
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/idt.h>

#include <core/mm/pmm.h>
#include <core/mm/vmm.h>

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
    
    // GDT

    x86_64_gdt_load();
    x86_64_idt_load();

    asm ("int $18");

    // PMM

    pmm_init();

    // VMM

    vmm_init();

    log("KERNEL END");
    cpu_halt();
}