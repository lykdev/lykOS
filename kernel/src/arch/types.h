#pragma once

#include <lib/def.h>

#if defined (__x86_64__)

typedef enum page_size : u64
{
    PAGE_SIZE_4KIB = KIB * 4,
    PAGE_SIZE_2MIB = MIB * 2,
    PAGE_SIZE_1GIB = GIB
} page_size_t;

typedef struct
{
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 rbx;
    u64 rdx;
    u64 rcx;
    u64 rax;
} __attribute__((packed)) regs_t;

#else
    #error Unimplemented.
#endif