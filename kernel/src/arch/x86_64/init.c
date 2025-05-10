#include <arch/x86_64/devices/ioapic.h>
#include <arch/x86_64/devices/lapic.h>
#include <arch/x86_64/devices/pic.h>
#include <arch/x86_64/tables/gdt.h>
#include <arch/x86_64/tables/idt.h>
#include <arch/x86_64/fpu.h>

void arch_init()
{
    x86_64_idt_make();
    x86_64_pic_disable();
    x86_64_ioapic_init();
    x86_64_fpu_init();
}

void arch_cpu_core_init()
{
    x86_64_gdt_load();
    x86_64_idt_load();
    x86_64_fpu_init_cpu();
    x86_64_lapic_init();
}
