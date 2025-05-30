#pragma once

#include <lib/def.h>

#if defined(__x86_64__)

#define ARCH_KERNEL_MIN_VIRT 0xFFFF800000000000ull
#define ARCH_KERNEL_MAX_VIRT 0xFFFFFFFFFFFFFFFFull

#define ARCH_USER_MIN_VIRT 0x0000000000000000ull
#define ARCH_USER_MAX_VIRT 0x00007FFFFFFFFFFFull

#define ARCH_PAGE_SIZE_4K 0x1000ull
#define ARCH_PAGE_SIZE_2M 0x200000ull
#define ARCH_PAGE_SIZE_1G 0x40000000ull
#define ARCH_PAGE_GRAN ARCH_PAGE_SIZE_4K

#define ARCH_PAGE_SIZES ((u64[]){ARCH_PAGE_SIZE_4K, ARCH_PAGE_SIZE_2M, ARCH_PAGE_SIZE_1G})
#define ARCH_PAGE_SIZES_LEN 3

#elif defined(__aarch64__)

#define ARCH_KERNEL_MIN_VIRT 0xFFFF800000000000ull
#define ARCH_KERNEL_MAX_VIRT 0xFFFFFFFFFFFFFFFFull

#define ARCH_USER_MIN_VIRT 0x0000000000000000ull
#define ARCH_USER_MAX_VIRT 0x00007FFFFFFFFFFFull

#define ARCH_PAGE_SIZE_4K 0x1000ull
#define ARCH_PAGE_SIZE_2M 0x200000ull
#define ARCH_PAGE_SIZE_1G 0x40000000ull
#define ARCH_PAGE_GRAN ARCH_PAGE_SIZE_4K

#define ARCH_PAGE_SIZES ((u64[]){ARCH_PAGE_SIZE_4K, ARCH_PAGE_SIZE_2M, ARCH_PAGE_SIZE_1G})
#define ARCH_PAGE_SIZES_LEN 3

#endif
