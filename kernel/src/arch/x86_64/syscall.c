#include <arch/syscall.h>
#include <arch/x86_64/msr.h>
#include <arch/x86_64/tables/gdt.h>

extern void arch_syscall_entry();

void arch_syscall_init() {
  // Enable SYSCALL/SYSRET by setting SCE (System Call Extensions) bit.
  x86_64_msr_write(X86_64_MSR_EFER, x86_64_msr_read(X86_64_MSR_EFER) | 1);

  // Set up STAR (segments for SYSCALL/SYSRET).
  // Read the AMD spec to understand why substracting 8 is needed here.
  x86_64_msr_write(X86_64_MSR_STAR,
                   ((u64)(X86_64_GDT_SELECTOR_DATA64_RING3 - 8) << 48) |
                       ((u64)X86_64_GDT_SELECTOR_CODE64_RING0 << 32));

  // Set up LSTAR (SYSCALL handler entry point).
  x86_64_msr_write(X86_64_MSR_LSTAR, (u64)arch_syscall_entry);

  // Set up SFMASK (RFLAGS bits that should be cleared during SYSCALL).
  // Disable interrupts (IF=0).
  x86_64_msr_write(X86_64_MSR_SFMASK,
                   x86_64_msr_read(X86_64_MSR_SFMASK) | (1 << 9));
}
