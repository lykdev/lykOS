#include <arch/x86_64/tables/gdt.h>
#include <arch/x86_64/tables/idt.h>

void arch_cpu_core_init()
{
  x86_64_gdt_load();
  x86_64_idt_load();
}
