#include <limine.h>

#include <arch/cpu.h>

#include <core/mm/pmm.h>
#include <core/mm/vmm.h>

#include <lib/def.h>
#include <lib/log.h>
#include <lib/assert.h>

__attribute__((used))
static volatile LIMINE_BASE_REVISION(1);

void _entry()
{
    log("KERNEL START");

    // Ensure the bootloader actually understands our base revision.
    ASSERT(LIMINE_BASE_REVISION_SUPPORTED != false);
    
    pmm_init();
    vmm_init();

    log("KERNEL END");
    cpu_halt();
}