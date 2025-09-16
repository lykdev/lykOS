#pragma once

#include <lib/def.h>

typedef struct
{
    u16 vendor_id;
    u16 device_id;
    u16 command;
    u16 status;
    u8  revision_id;
    u8  prog_if;
    u8  subclass;
    u8  class;
    u8  cache_line_size;
    u8  latency_timer;
    u8  header_type;
    u8  bist;
}
__attribute__((packed))
pci_header_common_t;

typedef struct
{
    pci_header_common_t common;
    u32 bar[6];
    u32 cardbus_cis_ptr;
    u16 subsystem_vendor_id;
    u16 subsystem_id;
    u32 expansion_rom_base;
    u8  capabilities_ptr;
    u8  _rsv[7];
    u8  interrupt_line;
    u8  interrupt_pin;
    u8  min_grant;
    u8  max_latency;
}
__attribute__((packed))
pci_header_type0_t;

typedef struct
{
    pci_header_common_t common;
    u32 bar[2];
    u8  primary_bus;
    u8  secondary_bus;
    u8  subordinate_bus;
    u8  secondary_latency_timer;
    u8  io_base;
    u8  io_limit;
    u16 secondary_status;
    u16 memory_base;
    u16 memory_limit;
    u16 prefetchable_memory_base;
    u16 prefetchable_memory_limit;
    u32 prefetchable_base_upper32;
    u32 prefetchable_limit_upper32;
    u16 io_base_upper16;
    u16 io_limit_upper16;
    u8  capabilities_ptr;
    u8  reserved1[3];
    u32 expansion_rom_base;
    u8  interrupt_line;
    u8  interrupt_pin;
    u16 bridge_control;
}
__attribute__((packed))
pci_header_type1_t;

typedef struct
{
    pci_header_common_t common;
    u32 bar[1];
    u8  capabilities_ptr;
    u8  _rsv1[3];
    u32 expansion_rom_base;
    u8  interrupt_line;
    u8  interrupt_pin;
    u8  _rsv2[2];
    u32 socket_base;
    u8  offset_capabilities;
    u8  _rsv3[3];
    u32 legacy_mode_base;
}
__attribute__((packed))
pci_header_type2_t;

void pci_init();
