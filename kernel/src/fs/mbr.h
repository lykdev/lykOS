#pragma once

#include <lib/def.h>

typedef struct
{
    u8  attributes;
    u8  chs_addr_start[3];
    u8  type;
    u8  chs_addr_end[3];
    u32 lba_start;
    u32 sector_count;
}
__attribute((packed))
mbr_part_table_entry_t;

typedef struct
{
    u8  bootstrap[440];
    u32 udid;
    u16 _rsv;
    mbr_part_table_entry_t part_table[4];
    u16 signature; // 0xAA55
}
__attribute((packed))
mbr_header_t;
