#pragma once

#include <lib/def.h>
#include <lib/list.h>

extern list_t pci_unit_list;

typedef struct
{
    u16 vendor_id;
    u16 device_id;
    u16 command;
    u16 status;
    u8 rev_id;
    u8 prog_interface;
    u8 subclass;
    u8 class;
    u8 cache_lise_size;
    u8 latency_timer;
    u8 header_type;
    u8 bist;
} __attribute__((packed)) pci_common_hdr_t;

typedef struct
{
    pci_common_hdr_t *hdr;

    list_node_t list_element;
} pci_unit_t;

void pci_init();
