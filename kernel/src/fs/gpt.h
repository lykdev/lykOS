#pragma once

#include <lib/def.h>

typedef struct
{
    char signature[8]; // "EFI PART"
    u32  revision;
    u32  header_size;
    u32  header_crc32; // Header CRC32 checksum.
    u32  _rsv;
    u64  current_lba;
    u64  backup_lba;
    u64  first_usable_lba;
    u64  last_usable_lba;
    u8   guid[16];
    u64  part_table_lba;
    u32  part_table_entry_count;
    u32  part_table_entry_size;
    u32  part_table_crc32;
}
__attribute__((packed))
gpt_header_t;

typedef struct
{
    u8  type_guid[16]; // Partition type GUID.
    u8  guid[16];      // Partition GUID.
    u64 lba_start;
    u64 lba_end;
    u64 attributes;
    u16 partition_name[36]; // UTF-16LE, each character consumes 2 bytes.
}
__attribute__((packed))
gpt_part_table_entry_t;
