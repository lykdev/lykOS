#pragma once

#include <lib/def.h>

typedef struct
{
    enum
    {
        ECX,
        EDX
    }
    reg;
    u8 offset;
}
x86_64_cpuid_feature_t;

typedef struct
{
    u32 eax, ebx, ecx, edx;
}
x86_64_cpuid_response_t;

#define X86_64_CPUID_FEATURE_FPU          ((x86_64_cpuid_feature_t) {EDX, 0})
#define X86_64_CPUID_FEATURE_VME          ((x86_64_cpuid_feature_t) {EDX, 1})
#define X86_64_CPUID_FEATURE_DE           ((x86_64_cpuid_feature_t) {EDX, 2})
#define X86_64_CPUID_FEATURE_PSE          ((x86_64_cpuid_feature_t) {EDX, 3})
#define X86_64_CPUID_FEATURE_TSC          ((x86_64_cpuid_feature_t) {EDX, 4})
#define X86_64_CPUID_FEATURE_MSR          ((x86_64_cpuid_feature_t) {EDX, 5})
#define X86_64_CPUID_FEATURE_PAE          ((x86_64_cpuid_feature_t) {EDX, 6})
#define X86_64_CPUID_FEATURE_MCE          ((x86_64_cpuid_feature_t) {EDX, 7})
#define X86_64_CPUID_FEATURE_CX8          ((x86_64_cpuid_feature_t) {EDX, 8})
#define X86_64_CPUID_FEATURE_APIC         ((x86_64_cpuid_feature_t) {EDX, 9})
#define X86_64_CPUID_FEATURE_MTRR         ((x86_64_cpuid_feature_t) {EDX, 10})
#define X86_64_CPUID_FEATURE_SEP          ((x86_64_cpuid_feature_t) {EDX, 11})
#define X86_64_CPUID_FEATURE_MTRR2        ((x86_64_cpuid_feature_t) {EDX, 12})
#define X86_64_CPUID_FEATURE_PGE          ((x86_64_cpuid_feature_t) {EDX, 13})
#define X86_64_CPUID_FEATURE_MCA          ((x86_64_cpuid_feature_t) {EDX, 14})
#define X86_64_CPUID_FEATURE_CMOV         ((x86_64_cpuid_feature_t) {EDX, 15})
#define X86_64_CPUID_FEATURE_PAT          ((x86_64_cpuid_feature_t) {EDX, 16})
#define X86_64_CPUID_FEATURE_PSE36        ((x86_64_cpuid_feature_t) {EDX, 17})
#define X86_64_CPUID_FEATURE_PSN          ((x86_64_cpuid_feature_t) {EDX, 18})
#define X86_64_CPUID_FEATURE_CLFSH        ((x86_64_cpuid_feature_t) {EDX, 19})
#define X86_64_CPUID_FEATURE_NX           ((x86_64_cpuid_feature_t) {EDX, 20})
#define X86_64_CPUID_FEATURE_DS           ((x86_64_cpuid_feature_t) {EDX, 21})
#define X86_64_CPUID_FEATURE_ACPI         ((x86_64_cpuid_feature_t) {EDX, 22})
#define X86_64_CPUID_FEATURE_MMX          ((x86_64_cpuid_feature_t) {EDX, 23})
#define X86_64_CPUID_FEATURE_FXSR         ((x86_64_cpuid_feature_t) {EDX, 24})
#define X86_64_CPUID_FEATURE_SSE          ((x86_64_cpuid_feature_t) {EDX, 25})
#define X86_64_CPUID_FEATURE_SSE2         ((x86_64_cpuid_feature_t) {EDX, 26})
#define X86_64_CPUID_FEATURE_SS           ((x86_64_cpuid_feature_t) {EDX, 27})
#define X86_64_CPUID_FEATURE_HTT          ((x86_64_cpuid_feature_t) {EDX, 28})
#define X86_64_CPUID_FEATURE_TM           ((x86_64_cpuid_feature_t) {EDX, 29})
#define X86_64_CPUID_FEATURE_IA64         ((x86_64_cpuid_feature_t) {EDX, 30})
#define X86_64_CPUID_FEATURE_PBE          ((x86_64_cpuid_feature_t) {EDX, 31})

#define X86_64_CPUID_FEATURE_SSE3         ((x86_64_cpuid_feature_t) {ECX, 0})
#define X86_64_CPUID_FEATURE_PCLMULQDQ    ((x86_64_cpuid_feature_t) {ECX, 1})
#define X86_64_CPUID_FEATURE_DTES64       ((x86_64_cpuid_feature_t) {ECX, 2})
#define X86_64_CPUID_FEATURE_MONITOR      ((x86_64_cpuid_feature_t) {ECX, 3})
#define X86_64_CPUID_FEATURE_DS_CPL       ((x86_64_cpuid_feature_t) {ECX, 4})
#define X86_64_CPUID_FEATURE_VMX          ((x86_64_cpuid_feature_t) {ECX, 5})
#define X86_64_CPUID_FEATURE_SMX          ((x86_64_cpuid_feature_t) {ECX, 6})
#define X86_64_CPUID_FEATURE_EST          ((x86_64_cpuid_feature_t) {ECX, 7})
#define X86_64_CPUID_FEATURE_TM2          ((x86_64_cpuid_feature_t) {ECX, 8})
#define X86_64_CPUID_FEATURE_SSSE3        ((x86_64_cpuid_feature_t) {ECX, 9})
#define X86_64_CPUID_FEATURE_CNXT_ID      ((x86_64_cpuid_feature_t) {ECX, 10})
#define X86_64_CPUID_FEATURE_SDBG         ((x86_64_cpuid_feature_t) {ECX, 11})
#define X86_64_CPUID_FEATURE_FMA          ((x86_64_cpuid_feature_t) {ECX, 12})
#define X86_64_CPUID_FEATURE_CX16         ((x86_64_cpuid_feature_t) {ECX, 13})
#define X86_64_CPUID_FEATURE_XTPR         ((x86_64_cpuid_feature_t) {ECX, 14})
#define X86_64_CPUID_FEATURE_PDCM         ((x86_64_cpuid_feature_t) {ECX, 15})
#define X86_64_CPUID_FEATURE_RESERVED1    ((x86_64_cpuid_feature_t) {ECX, 16})
#define X86_64_CPUID_FEATURE_PCID         ((x86_64_cpuid_feature_t) {ECX, 17})
#define X86_64_CPUID_FEATURE_DCA          ((x86_64_cpuid_feature_t) {ECX, 18})
#define X86_64_CPUID_FEATURE_SSE4_1       ((x86_64_cpuid_feature_t) {ECX, 19})
#define X86_64_CPUID_FEATURE_SSE4_2       ((x86_64_cpuid_feature_t) {ECX, 20})
#define X86_64_CPUID_FEATURE_X2APIC       ((x86_64_cpuid_feature_t) {ECX, 21})
#define X86_64_CPUID_FEATURE_MOVBE        ((x86_64_cpuid_feature_t) {ECX, 22})
#define X86_64_CPUID_FEATURE_POPCNT       ((x86_64_cpuid_feature_t) {ECX, 23})
#define X86_64_CPUID_FEATURE_TSC_DEADLINE ((x86_64_cpuid_feature_t) {ECX, 24})
#define X86_64_CPUID_FEATURE_AES_NI       ((x86_64_cpuid_feature_t) {ECX, 25})
#define X86_64_CPUID_FEATURE_XSAVE        ((x86_64_cpuid_feature_t) {ECX, 26})
#define X86_64_CPUID_FEATURE_OSXSAVE      ((x86_64_cpuid_feature_t) {ECX, 27})
#define X86_64_CPUID_FEATURE_AVX          ((x86_64_cpuid_feature_t) {ECX, 28})
#define X86_64_CPUID_FEATURE_F16C         ((x86_64_cpuid_feature_t) {ECX, 29})
#define X86_64_CPUID_FEATURE_RDRND        ((x86_64_cpuid_feature_t) {ECX, 30})
#define X86_64_CPUID_FEATURE_HYPERVISOR   ((x86_64_cpuid_feature_t) {ECX, 31})

bool x86_64_cpuid_check_feature(x86_64_cpuid_feature_t feature);

x86_64_cpuid_response_t x86_64_cpuid_get_reg(u32 eax, u32 ecx);
