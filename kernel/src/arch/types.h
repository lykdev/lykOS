#pragma once

#include <lib/def.h>

#if defined (__x86_64__)

typedef enum page_size : u64
{
    PAGE_SIZE_4KIB = KIB * 4,
    PAGE_SIZE_2MIB = MIB * 2,
    PAGE_SIZE_1GIB = GIB
} page_size_t;

#else
    #error Unimplemented.
#endif