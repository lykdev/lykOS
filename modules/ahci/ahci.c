#include "ahci.h"

#include <common/hhdm.h>
#include <common/log.h>
#include <dev/block.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

static void ahci_port_idle(port_t* port)
{
    // PxCMD.ST, PxCMD.CR, PxCMD.FRE, PxCMD.FR should all be cleared.
    const u32 idle_flags_mask = (u32)1 << 15  // CR
                              | (u32)1 << 14  // FR
                              | (u32)1 <<  4  // FRE
                              | (u32)1 <<  0; // ST
    if ((port->cmd & idle_flags_mask) != 0)
    {
        port->cmd &= ~((u32)1 << 0); // Clear PxCMD.ST
        while ((port->cmd & ((u32)1 << 15)) != 0) // Wait for PxCMD.CR to return ‘0’ when read.
            ;

        port->cmd &= ~((u32)1 << 4); // Clear PxCMD.FRE
        while ((port->cmd & ((u32)1 << 15)) != 0) // Wait for PxCMD.FR to return ‘0’ when read.
            ;
    }
}

static void ahci_port_start(port_t* port)
{
    while ((port->cmd & ((u32)1 << 15)) != 0) // Wait for PxCMD.CR to return ‘0’ when read.
        ;

    port->cmd |= (u32)1 << 0; // Set PxCMD.ST
    port->cmd |= (u32)1 << 4; // Set PxCMD.FRE
}

static uint ahci_find_free_cmd_slot(port_t* port)
{
    u32 slots = port->sact | port->ci;
    for (uint i = 0; i < 32; i++)
        if ((slots & (1 << i)) == 0)
            return i;

    return -1;
}

static bool ahci_issue_cmd(port_t *port, int slot)
{
    port->is = (u32)-1; // Clear port interrupt.
    while ((port->tfd & (1 << 7 | 1 << 3)) != 0) // Wait until the port is not busy.
        ;
    port->ci |= 1 << slot; // Issue command.
    while ((port->ci & (1 << slot)) != 0) // Wait for completion.
    {
        if (port->is & (1 << 30))
            return false;
    }

    return true;
}

bool ahci_read(block_device_t *blk_dev, u64 lba, void *buf, u64 count);
bool ahci_write(block_device_t *blk_dev, u64 lba, void *buf, u64 count);

static bool ahci_setup_storage_device(port_t *port)
{
    int slot = ahci_find_free_cmd_slot(port);
    if (slot == -1)
        return false;

    // Output buffer.
    uptr buf_phys = (uptr)pmm_alloc(0); // TODO: free

    cmd_header_t *cmd_list = (cmd_header_t*)((((u64)port->clbu << 32) | port->clb) + HHDM);
    cmd_header_t *cmd_hdr  = &cmd_list[slot];
    cmd_table_t  *cmd_tbl  = (cmd_table_t*)((((u64)cmd_hdr->ctbau << 32) | cmd_hdr->ctba) + HHDM);

    // Setup command header.
    cmd_hdr->cfl = sizeof(fis_reg_h2d_t) / 4; // Command FIS length in DWORDS.
    cmd_hdr->w = 0;     // Read.
    cmd_hdr->prdtl = 1; // One PRDT entry.
    // Setup command table.
    fis_reg_h2d_t* fis = (fis_reg_h2d_t *)&cmd_tbl->cfis;
    *fis = (fis_reg_h2d_t) {
        .type = FIS_TYPE_REG_H2D,
        .c = 1,             // Command.
        .command = 0xEC     // IDENTIFY DEVICE (defined in the ATA/ATAPI spec).
    };
    memset(cmd_tbl->acmd, 0, sizeof(cmd_tbl->acmd));
    cmd_tbl->prdt_entry[0] = (prdt_entry_t) {
        .dba  = buf_phys & 0xFFFFFFFF,
        .dbau = (buf_phys >> 32) & 0xFFFFFFFF,
        .dbc  = 512 - 1, // Byte count.
        .i = 1
    };

    if (!ahci_issue_cmd(port, slot))
        return false;

    u16 *identify_buf = (u16 *)(buf_phys + HHDM);
    char *name = heap_alloc(41);
    for (int i = 0; i < 20; i++)
    {
        name[i * 2]     = identify_buf[27 + i] >> 8;
        name[i * 2 + 1] = identify_buf[27 + i] & 0xFF;
    }
    name[40] = '\0';

    // (identify_buf[217] == 0) ? STORAGE_DEVICE_SSD : STORAGE_DEVICE_HDD,
    u64 sector_size  = identify_buf[106] & (1 << 12) ?
                       ((u32)identify_buf[118] << 16) | identify_buf[117]
                     : 512;
    u64 sector_count = ((u64)identify_buf[61] << 16) | identify_buf[60];

    block_device_register_drive(
        name,
        sector_size,
        sector_count,
        ahci_read,
        ahci_write,
        (void *)port
    );

    return true;
}

void ahci_setup(uptr abar)
{
    generic_host_control_t* ghc = (generic_host_control_t*)(abar + HHDM);

    // Indicate that system software is AHCI aware.
    ghc->ghc = 1 << 31; // Set GHC.AE to 1.

    // Number of command slots for each supported port.
    u32 cmd_slots = (ghc->cap >> 8) & 0x1F;
    // Indicates whether the HBA can access 64-bit data structures.
    bool ext_addr = (ghc->cap >> 31) & 1;
    if(!ext_addr)
    {
        log("AHCI: HBA cannot access 64-bit data structures.");
        return;
    }

    for (int i = 0; i < 32; i++)
    {
        if (((ghc->pi >> i) & 1) == 0) // Bit N is set to 1 if port N is implemented.
            continue;
        port_t* port = (port_t*)((uptr)ghc + 0x100 + (i * 0x80));
        if (port->sig != 0x00000101) // Skip non-SATA drives.
            continue;

        // Ensure the port is in an idle state prior to manipulating HBA and port specific registers.
        ahci_port_idle(port);

        // Allocate mmeory for the Command List Structure.
        u64 clb = (u64)pmm_alloc(0);
        port->clb = clb & 0xFFFFFFFF;
    	port->clbu = (clb >> 32) & 0xFFFFFFFF;
    	memset((void*)(clb + HHDM), 0, 1024);

        // Allocate mmeory for the Received FIS Structure.
        u64 fb = (u64)pmm_alloc(0);
    	port->fb = fb & 0xFFFFFFFF;
    	port->fbu = (fb >> 32) & 0xFFFFFFFF;
    	memset((void*)(fb + HHDM), 0, 256);

        cmd_header_t* cmd_list = (cmd_header_t*)(u64)(clb + HHDM);
        for (uint slot = 0; slot < cmd_slots; slot++)
        {
            cmd_header_t *cmd_hdr = &cmd_list[slot];

            cmd_hdr->prdtl = 0; // No PRDTs yet.

            u64 ctba = (u64)pmm_alloc(0); // Must be 128-byte aligned. TODO: swap for HEAP.
            cmd_hdr->ctba = ctba & 0xFFFFFFFF;
            cmd_hdr->ctbau = (ctba >> 32) & 0xFFFFFFFF;

            memset((void*)(ctba + HHDM), 0, sizeof(cmd_table_t));
        }

        ahci_port_start(port);

        if (!ahci_setup_storage_device(port))
            continue;
    }
}

bool ahci_read(block_device_t *blk_dev, u64 lba, void *buf, u64 count)
{
    port_t *port = blk_dev->driver_data;

    int slot = ahci_find_free_cmd_slot(port);
    if (slot == -1)
        return false;

    u64 buf_phys = vmm_virt_to_phys(g_vmm_kernel_addr_space, (uptr)buf);

    cmd_header_t *cmd_list = (cmd_header_t*)((((u64)port->clbu << 32) | port->clb) + HHDM);
    cmd_header_t *cmd_hdr  = &cmd_list[slot];
    cmd_table_t  *cmd_tbl  = (cmd_table_t*)((((u64)cmd_hdr->ctbau << 32) | cmd_hdr->ctba) + HHDM);

    // Setup command header.
    cmd_hdr->cfl = sizeof(fis_reg_h2d_t) / 4; // Command FIS length in DWORDS.
    cmd_hdr->w = 0;     // Read.
    cmd_hdr->prdtl = 1; // One PRDT entry.
    // Setup command table.
    fis_reg_h2d_t* fis = (fis_reg_h2d_t *)&cmd_tbl->cfis;
    *fis = (fis_reg_h2d_t) {
        .type = FIS_TYPE_REG_H2D,
        .c = 1,                     // Command.
        .command = 0x25,            // READ DMA EXT (defined in the ATA/ATAPI spec).
        .lba0 = lba & 0xFF,
        .lba1 = (lba >>  8) & 0xFF,
        .lba2 = (lba >> 16) & 0xFF,
        .device = 1 << 6,           // LBA mode.
        .lba3 = (lba >> 24) & 0xFF,
        .lba4 = (lba >> 32) & 0xFF,
        .lba5 = (lba >> 40) & 0xFF,
        .count0 = count & 0xFF,
        .count1 = (count >> 8) & 0xFF
    };
    memset(cmd_tbl->acmd, 0, sizeof(cmd_tbl->acmd));
    cmd_tbl->prdt_entry[0] = (prdt_entry_t) {
        .dba =  buf_phys & 0xFFFFFFFF,
        .dbau = (buf_phys >> 32) & 0xFFFFFFFF,
        .dbc =  (count * 512) - 1, // Byte count.
        .i = 1 // Trigger interrupt on completion.
    };

    if (!ahci_issue_cmd(port, slot))
        return false;

    return true;
}

bool ahci_write(block_device_t *blk_dev, u64 lba, void *buf, u64 count)
{
    return false; // TODO: imlement ahci_write.
}
