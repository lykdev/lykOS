#pragma once

#include <lib/utils.h>

struct acpi_std_hdr
{
    char signature[4];
    u32 length;
    u8 revision;
    u8 checksum;
    char oem_id[6];
    char oem_table_id[8];
    u32 oem_revision;
    u32 creator_id;
    u32 creator_revision;
} __attribute__((packed));

void acpi_init();

struct acpi_std_hdr* acpi_sysdesc_lookup(char *signature);