#pragma once

#include <lib/def.h>

typedef struct
{
    u32 rsv0;
    u32 rsp0_lower;
    u32 rsp0_upper;
    u32 rsp1_lower;
    u32 rsp1_upper;
    u32 rsp2_lower;
    u32 rsp2_upper;
    u32 rsv1;
    u32 rsv2;
    u32 ist1_lower;
    u32 ist1_upper;
    u32 ist2_lower;
    u32 ist2_upper;
    u32 ist3_lower;
    u32 ist3_upper;
    u32 ist4_lower;
    u32 ist4_upper;
    u32 ist5_lower;
    u32 ist5_upper;
    u32 ist6_lower;
    u32 ist6_upper;
    u32 ist7_lower;
    u32 ist7_upper;
    u32 rsv3;
    u32 rsv4;
    u16 rsv5;
    u16 iomap_base;
}
__attribute__((packed)) x86_64_tss_t;

/**
 * @brief Set the CPL0 stack pointer.
 */
void x86_64_tss_set_rsp0(x86_64_tss_t *tss, uptr stack_pointer);
