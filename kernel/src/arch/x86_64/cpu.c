#include <arch/cpu.h>
#include <arch/x86_64/msr.h>

#include <common/log.h>

void *arch_cpu_read_thread_reg() {
  void *thread = NULL;
  asm volatile("mov %%gs:0, %0" : "=r"(thread));

  return thread;
}

void arch_cpu_write_thread_reg(void *t) {
  x86_64_msr_write(X86_64_MSR_GS_BASE, (u64)t);
}

void arch_cpu_halt() {
  while (true)
    asm volatile("hlt");
}

void arch_cpu_relax() { asm volatile("pause"); }
