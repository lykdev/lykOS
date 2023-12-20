#include "cpu.h"

#include <lib/utils.h>

void cpu_core_entry(struct limine_smp_info *lcore)
{
    // A pointer to the limine_smp_info structure of the CPU is passed in RDI by the bootloader.
    // RDI being the first argument. Check the System V ABI.

    log("Initializing CPU core: LAPIC ID %d, ACPI Processor UID %d", lcore->lapic_id, lcore->processor_id);

    halt();
}