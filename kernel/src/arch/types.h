#pragma once

#include <lib/def.h>

#if defined(__x86_64__)

#define ARCH_MAX_VIRT_ADDR (((u64)1 << 48) - 1)
#define ARCH_HIGHER_HALF_START 0xFFFF800000000000ull

#define ARCH_PAGE_SIZE_4K 0x1000ull
#define ARCH_PAGE_SIZE_2M 0x200000ull
#define ARCH_PAGE_SIZE_1G 0x40000000ull
#define ARCH_PAGE_GRAN ARCH_PAGE_SIZE_4K

#define ARCH_PAGE_SIZES                                                        \
  ((u64[]){ARCH_PAGE_SIZE_4K, ARCH_PAGE_SIZE_2M, ARCH_PAGE_SIZE_1G})
#define ARCH_PAGE_SIZES_LEN 3

#elif defined(__aarch64__)

#define ARCH_MAX_VIRT_ADDR (((u64)1 << 48) - 1)
#define ARCH_HIGHER_HALF_START 0xFFFF800000000000ull

#define ARCH_PAGE_SIZE_4K 0x1000ull
#define ARCH_PAGE_SIZE_2M 0x200000ull
#define ARCH_PAGE_SIZE_1G 0x40000000ull
#define ARCH_PAGE_GRAN ARCH_PAGE_SIZE_4K

#define ARCH_PAGE_SIZES                                                        \
  ((u64[]){ARCH_PAGE_SIZE_4K, ARCH_PAGE_SIZE_2M, ARCH_PAGE_SIZE_1G})
#define ARCH_PAGE_SIZES_LEN 3

#endif
