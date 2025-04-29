#include "cpuid.h"
#include "common/assert.h"

bool x86_64_cpuid_check_feature(x86_64_cpuid_feature_t feature)
{
    u32 value;

    u32 eax = 0, ebx = 0, ecx = 0, edx = 0;
    asm volatile("cpuid" : "=c"(ecx), "=d"(edx) : "a"(1), "c"(0));
    switch(feature.reg)
    {
        case ECX: value = ecx;  break;
        case EDX: value = edx;  break;
        default: ASSERT(false); break;
    }

    return (bool)(value & (1 << feature.offset));
}

x86_64_cpuid_response_t x86_64_cpuid_get_reg(u32 eax, u32 ecx)
{
    x86_64_cpuid_response_t ret;

    asm volatile("cpuid" : "=a"(ret.eax), "=b"(ret.ebx), "=c"(ret.ecx), "=d"(ret.edx) : "a"(eax), "c"(ecx));

    return ret;
}
