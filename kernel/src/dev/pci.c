#include "pci.h"

#include <common/hhdm.h>
#include <common/log.h>
#include <common/panic.h>
#include <dev/acpi/acpi.h>
#include <fs/sysfs.h>
#include <lib/printf.h>
#include <lib/string.h>

typedef struct
{
    acpi_sdt_t sdt;
    u64 _rsv;
    struct
    {
        u64 base_addr;
        u16 segment_group;
        u8  bus_start;
        u8  bus_end;
        u32 _rsv;
    }
    __attribute__((packed))
    segments[];
}
__attribute__((packed))
acpi_mcfg_t;

static u64 get_hdr_size(u8 header_type)
{
    switch (header_type & 0b11)
    {
    case 0x0:
        return sizeof(pci_header_type0_t);
    case 0x1:
        return sizeof(pci_header_type1_t);
    case 0x2:
        return sizeof(pci_header_type2_t);
    default:
        panic("PCI: What header type is this lol?");
    }
}

static void setup_pci_dir_tree()
{
    vnode_t *dev_dir, *bus_dir, *pci_dir, *devices_dir;

    // Ensure `/dev` exists.
    if (vfs_open("/dev", &dev_dir) != EOK)
        panic("Could not open '/dev'!");

    // Create `/dev/bus` if needed.
    if (dev_dir->ops->lookup(dev_dir, "bus", &bus_dir) != EOK)
        if (dev_dir->ops->create(dev_dir, "bus", VFS_NODE_DIR, &bus_dir) != EOK)
            panic("Could not create `/dev/bus`!");

    // Create `/dev/bus/pci`.
    if (bus_dir->ops->create(bus_dir, "pci", VFS_NODE_DIR, &pci_dir) != EOK)
        panic("Could not create `/dev/bus/pci`!");

    // Create `/dev/bus/pci/devices`.
    if (pci_dir->ops->create(pci_dir, "devices", VFS_NODE_DIR, &devices_dir) != EOK)
        panic("Could not create `/dev/bus/pci/devices`!");

    vfs_close(devices_dir);
    vfs_close(pci_dir);
    vfs_close(bus_dir);
    vfs_close(dev_dir);
}


void pci_list()
{
    acpi_mcfg_t *mcfg = (acpi_mcfg_t*)acpi_lookup("MCFG");
    if (mcfg == NULL)
    {
        log("PCI: MCFG table not found!");
        return;
    }

    setup_pci_dir_tree();

    vnode_t *pci_dev_dir;
    vfs_open("/dev/bus/pci/devices", &pci_dev_dir);

    for (u64 i = 0; i < (mcfg->sdt.length - sizeof(acpi_mcfg_t)) / 16; i++)
    {
        u64 base      = mcfg->segments[i].base_addr;
        u64 bus_start = mcfg->segments[i].bus_start;
        u64 bus_end   = mcfg->segments[i].bus_end;

        for (u64 bus = bus_start; bus <= bus_end; bus++)
            for (u64 dev = 0; dev < 32; dev++)
                for (u64 func = 0; func < 8; func++)
                {
                    pci_header_common_t *pci_hdr = (pci_header_common_t*)(HHDM + base + ((bus << 20) | (dev << 15) | (func << 12)));
                    if (pci_hdr->vendor_id == 0xFFFF)
                        continue;

                    // Create pci device file.
                    // Name format: VNID-DVID-CC:SS:PP
                    char name[32];
                    sprintf(name, "%04X:%04X-%02X:%02X:%02X",
                        pci_hdr->vendor_id, pci_hdr->device_id,
                        pci_hdr->class, pci_hdr->subclass, pci_hdr->prog_if
                    );

                    vnode_t *file;
                    if (pci_dev_dir->ops->create(pci_dev_dir, name, VFS_NODE_FILE, &file) != EOK)
                        panic("Could not create file for PCI device '%s'!", name);
                    file->mp_data = pci_hdr;
                    file->size = get_hdr_size(pci_dev_dir->type);
                    vfs_close(file);

                    log("PCI: %04X:%04X-%02X:%02X:%02X",
                        pci_hdr->vendor_id, pci_hdr->device_id,
                        pci_hdr->class, pci_hdr->subclass, pci_hdr->prog_if
                    );
                }
    }

    log("PCI: Successfully listed devices.");
}
