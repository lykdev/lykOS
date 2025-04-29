#include "fpu.h"

#include <arch/x86_64/cpuid.h>
#include <common/assert.h>

u32 g_x86_64_fpu_area_size = 0;
void (*g_x86_64_fpu_save)(void *area) = 0;
void (*g_x86_64_fpu_restore)(void *area) = 0;

static inline void fxsave(void *area)
{
    asm volatile("fxsave (%0)" : : "r"(area) : "memory");
}

static inline void fxrstor(void *area)
{
    asm volatile("fxrstor (%0)" : : "r"(area) : "memory");
}

static inline void xsave(void *area)
{
    asm volatile("xsave (%0)" : : "r"(area), "a"(0xFFFF'FFFF), "d"(0xFFFF'FFFF) : "memory");
}

static inline void xrstor(void *area)
{
    asm volatile("xrstor (%0)" : : "r"(area), "a"(0xFFFF'FFFF), "d"(0xFFFF'FFFF) : "memory");
}

void x86_64_fpu_init()
{
    if (!x86_64_cpuid_check_feature(X86_64_CPUID_FEATURE_FXSR))
        panic("FPU: FXSAVE and FXRSTOR instructions are not supported!");

    if(x86_64_cpuid_check_feature(X86_64_CPUID_FEATURE_XSAVE))
    {
        u32 area_size = x86_64_cpuid_get_reg(0xD, 0).ecx;
        g_x86_64_fpu_area_size = area_size;
        g_x86_64_fpu_save = xsave;
        g_x86_64_fpu_restore = xrstor;
    }
    else
    {
        g_x86_64_fpu_area_size = 512;
        g_x86_64_fpu_save = fxsave;
        g_x86_64_fpu_restore = fxrstor;
    }
}

void x86_64_fpu_init_cpu()
{
    volatile u64 cr0, cr4;

    // Enable x87
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2); // CR0.EM
    cr0 |= 1 << 1;    // CR0.MP
    asm volatile ("mov %0, %%cr0" :: "r"(cr0) : "memory");

    // Enable MMX
    asm volatile ("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= 1 << 9;  // CR4.OSFXSR
    cr4 |= 1 << 10; // CR4.OSXMMEXCPT
    asm volatile ("mov %0, %%cr4" :: "r"(cr4) : "memory");

    if(x86_64_cpuid_check_feature(X86_64_CPUID_FEATURE_XSAVE))
    {
        asm volatile ("mov %%cr4, %0" : "=r"(cr4));
        cr4 |= 1 << 18; // CR4.OSXSAVE
        asm volatile ("mov %0, %%cr4" :: "r"(cr4) : "memory");

        u64 xcr0 = 0;
        xcr0 |= 1 << 0; // XCR0.X87
        xcr0 |= 1 << 1; // XCR0.SSE
        if(x86_64_cpuid_check_feature(X86_64_CPUID_FEATURE_AVX))
            xcr0 |= 1 << 2; // XCR0.AVX

        //TODO: AVX512 support

        asm volatile("xsetbv" : : "a"(xcr0), "d"(xcr0 >> 32), "c"(0) : "memory");
    }
}
