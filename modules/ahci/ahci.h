#pragma once

#include <lib/def.h>

typedef volatile struct
{
    u32 host_capabilities;
    u32 global_host_control;
    u32 interrupt_status;
    u32 ports_implemented;
    u32 version;
    u32 ccc_control;
    u32 ccc_ports;
    u32 em_location;
    u32 em_control;
    u32 host_capabilities_ext;
    u32 bios_os_handoff;
}
__attribute__((packed))
generic_host_control_t;

typedef volatile struct
{
    u32 command_list_base;
    u32 command_list_base_upper;
    u32 fis_base;
    u32 fis_base_upper;
    u32 interrupt_status;
    u32 interrupt_enable;
    u32 command_and_status;
    u32 _rsv0;
    u32 task_file_data;
    u32 signature;
    u32 sata_status;
    u32 sata_control;
    u32 sata_error;
    u32 sata_active;
    u32 command_issue;
    u32 sata_notification;
    u32 fis_switch_control;
    u32 device_sleep;
    u32 _rsv1[10];
    u32 vendor_specific[4];
}
__attribute__((packed))
port_t;

typedef enum
{
	FIS_TYPE_REG_H2D	= 0x27,
	FIS_TYPE_REG_D2H	= 0x34,
	FIS_TYPE_DMA_ACT	= 0x39,
	FIS_TYPE_DMA_SETUP	= 0x41,
	FIS_TYPE_DATA		= 0x46,
	FIS_TYPE_BIST		= 0x58,
	FIS_TYPE_PIO_SETUP	= 0x5F,
	FIS_TYPE_DEV_BITS	= 0xA1,
}
fis_type_t;

typedef struct
{
	// DWORD 0
	u8  fis_type;

	u8  pmport:4;
	u8  rsv0:3;
	u8  c:1;

	u8  command;
	u8  featurel;

	// DWORD 1
	u8  lba0;
	u8  lba1;
	u8  lba2;
	u8  device;

	// DWORD 2
	u8  lba3;
	u8  lba4;
	u8  lba5;
	u8  featureh;

	// DWORD 3
	u8  countl;
	u8  counth;
	u8  icc;
	u8  control;

	// DWORD 4
	u8  rsv1[4];
}
__attribute__((packed))
fis_reg_h2d;
