#include "gdt.h"

#include <lib/utils.h>

static u64 gdt_data[] = {
    0x0000000000000000,
    
    0x00009a000000ffff, // 16 code
    0x000093000000ffff, // 16 data

    0x00cf9a000000ffff, // 32 code
    0x00cf93000000ffff, // 32 data

    0x00af9b000000ffff, // 64 code
    0x00af93000000ffff, // 64 data

    0x00affb000000ffff, // 64 usermode code
    0x00aff3000000ffff  // 64 usermode data
};

struct gdt_desc
{
    u16 size;
    u64 offset;
} __attribute__((packed));

static struct gdt_desc gdtr;

void gdt_init()
{
    gdtr.offset = (u64)&gdt_data;
    gdtr.size = (u16)(sizeof(gdt_data) - 1);

    log("GDT initialized.");
}