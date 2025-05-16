#pragma once

#include <lib/def.h>

typedef volatile struct
{
    u32 cap;
    u32 ghc;
    u32 is;
    u32 pi;
    u32 vs;
    u32 ccc_ctl;
    u32 ccc_ports;
    u32 em_loc;
    u32 em_ctl;
    u32 cap2;
    u32 bohc;
}
__attribute__((packed))
generic_host_control_t;

typedef volatile struct
{
    u32 clb;
    u32 clbu;
    u32 fb;
    u32 fbu;
    u32 is;
    u32 ie;
    u32 cmd;
    u32 _rsv0;
    u32 tfd;
    u32 sig;
    u32 ssts;
    u32 sctl;
    u32 serr;
    u32 sact; // Serial ATA Active.
    u32 ci;   // Command Issue.
    u32 sntf;
    u32 fbs;
    u32 dsvslp;
    u32 _rsv1[10];
    u32 vs[4];
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
    u16 cfl : 5;
    u16 a : 1;
    u16 w : 1;
    u16 p : 1;
    u16 r : 1;
    u16 b : 1;
    u16 c : 1;
    u16 _rsv0 : 1;
    u16 pmp : 4;
    u16 prdtl;
    u32 prdbc;
    u16 ctba;
    u16 ctbau;
    u32 _rsv1[4];
}
__attribute__((packed))
cmd_header_t;

// Serial ATA AHCI - Physical Region Descriptor Table
typedef struct
{
	u32 dba;        // Data base address.
	u32 dbau;       // Data base address upper.
	u32 _rsv0;
	u32 dbc : 22;   // Byte count, 4MiB max.
	u32 _rsv1 : 9;
	u32 i : 1;		// Interrupt on completion.
}
__attribute__((packed))
prdt_entry_t;

typedef struct
{
	u8 cfis[64]; // Command FIS.
	u8 acmd[16]; // ATAPI command.
	u8 _rsv[48];
	prdt_entry_t prdt_entry[]; // Physical Region Descriptor Table, max 65535 entries.
}
__attribute__((packed))
cmd_table_t;

// Serial ATA - Register Host to Device FIS
typedef struct
{
    u8 type;
    u8 pmport : 4;
    u8 _rsv0 : 3;
    u8 c : 1;
    u8 command;
    u8 feature_low;
    u8 lba0;
    u8 lba1;
    u8 lba2;
    u8 device;
    u8 lba3;
    u8 lba4;
    u8 lba5;
    u8 feature_high;
    u8 count0;
    u8 count1;
    u8 isochronous_command_completion;
    u8 control;
    u8 rsv0[4];
}
__attribute__((packed))
fis_reg_h2d_t;

typedef struct
{
    u8 dma_setup[0x1C];
    u8 _rsv0[4];
    u8 pio_setup[0x14];
    u8 _rsv1[12];
    u8 d2h_register[0x14];
    u8 _rsv2[4];
    u8 set_device_bits[0x08];
    u8 unknown[64];
}
__attribute__((packed))
fis_recv_t;

void ahci_setup(uptr abar);
