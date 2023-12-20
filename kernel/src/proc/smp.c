#include "smp.h"

#include <lib/utils.h>

#if defined (__x86_64__)
    #include <arch/x86_64/cpu.h>
#endif

void smp_init()
{
    ASSERT_C(smp_request.response != NULL && smp_request.response->cpu_count > 0,
             "Invalid smp info provided by the bootloader.");

    for (u64 i = 0; i < smp_request.response->cpu_count; i++)
    {
        struct limine_smp_info *lcore = smp_request.response->cpus[i];
        
        // We skip the bootstrap logical core.
        if (lcore->lapic_id != smp_request.response->bsp_lapic_id)
            // An atomic write to this field causes the parked CPU to jump to the written address.
            lcore->goto_address = &cpu_core_entry;
    }
}
