// #include "smp.h"

// #include <arch/cpu.h>

// #include <utils/limine/requests.h>
// #include <utils/log.h>
// #include <utils/panic.h>

// void smp_init()
// {
//     if (request_smp.response == NULL)
//         panic("Invalid SMP info provided by the bootloader");

//     log("AA");

//     arch_cpu_core_count = 0;
//     for (size_t i = 0; i < request_smp.response->cpu_count; i++)
//     {
//         struct limine_smp_info *smp_info = request_smp.response->cpus[i];
        
//         //__atomic_store_n(&smp_info->goto_address, (limine_goto_address)&arch_cpu_core_init, __ATOMIC_SEQ_CST);
//     }
// }
