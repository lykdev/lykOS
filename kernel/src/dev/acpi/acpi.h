#pragma once

#include <lib/def.h>

typedef struct
{
    char signature[4];
    u32  length;
    u8   revision;
    u8   checksum;
    char oem_id[6];
    char oem_table_id[8];
    u32  oem_revision;
    u32  creator_id;
    u32  creator_revision;
}
__attribute__((packed))
acpi_sdt_t;

bool acpi_init();

acpi_sdt_t *acpi_lookup(const char *signature);
