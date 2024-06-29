#include "gdt.h"

#include <lib/def.h>
#include <lib/log.h>

typedef struct
{
    u16 limit_low;
    u16 base_low;
    u8  base_mid;
    u8  access;
    u8  limit_high : 4;
    u8  flags : 4;
    u8  base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct
{
    u16 size;
    u64 base;
} __attribute__((packed)) gdtr_t;

#define COMMON 0b10010011 // Present, Code or Data segment, Read-write, Accessed
#define KERNEL 0b00 << 5
#define USER   0b11 << 5
#define DATA   0b0  << 3
#define CODE   0b1  << 3

#define STR(S) #S
#define XSTR(S) STR(S)

gdt_entry_t gdt[] =
{
    // Null segment
    {},

    // 64-bit code ring 0.
    {
        .limit_low  = 0xFFFF,
        .base_low   = 0,
        .base_mid   = 0,
        .access     = COMMON | KERNEL | CODE,
        .limit_high = 0xF,
        .flags      = 0b1010,
        .base_high  = 0
    },

    // 64-bit data ring 0.
    {
        .limit_low  = 0xFFFF,
        .base_low   = 0,
        .base_mid   = 0,
        .access     = COMMON | KERNEL | DATA,
        .limit_high = 0xF,
        .flags      = 0b1010,
        .base_high  = 0
    },

    // User data ring 3.
    {
        .limit_low  = 0xFFFF,
        .base_low   = 0,
        .base_mid   = 0,
        .access     = COMMON | USER | DATA,
        .limit_high = 0xF,
        .flags      = 0b1010,
        .base_high  = 0
    },
    
    // User code ring 3.
    {
        .limit_low  = 0xFFFF,
        .base_low   = 0,
        .base_mid   = 0,
        .access     = COMMON | USER | CODE,
        .limit_high = 0xF,
        .flags      = 0b1010,
        .base_high  = 0
    },

    // TSS
    {}, {} 
};

void x86_64_gdt_load()
{
    gdtr_t gdtr;
    gdtr.size = sizeof(gdt) - 1;
    gdtr.base = (u64)&gdt;

    __asm__ volatile
    (
        "lgdt %0                                        \n"
        "push $" XSTR(X86_64_GDT_SEL_CODE_RING0)       "\n"
        "lea 1f(%%rip), %%rax                           \n"
        "push %%rax                                     \n"
        "lretq                                          \n"
        "1:                                             \n"
        "mov $" XSTR(X86_64_GDT_SEL_DATA_RING0) ", %%rax\n"
        "mov %%rax, %%ds                                \n"
        "mov %%rax, %%ss                                \n"
        "mov %%rax, %%es                                \n"
        : : "m" (gdtr) : "rax", "memory"
    );

    log("GDT loaded.");
}
