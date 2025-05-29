#include <arch/cpu.h>
#include <arch/int.h>

#include <common/log.h>
#include <lib/def.h>

extern void arch_int_setup();

void arch_int_handler(u64 source, u64 esr, u64 elr, u64 spsr, u64 far)
{
    log("INT: %llu %#llx %#llx %#llx %#llx", source, esr, elr, spsr, far);
    arch_cpu_halt();
}

void arch_int_init()
{
    arch_int_mask();
    arch_int_setup();
    arch_int_unmask();

    log("Loaded exception vector table.");
}
