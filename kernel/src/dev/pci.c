#include "pci.h"

#include <common/hhdm.h>
#include <common/log.h>
#include <common/panic.h>
#include <dev/acpi/acpi.h>
#include <fs/pfs.h>
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

static u64 read(vfs_node_t *self, u64 offset, void *buffer, u64 count)
{
    u64 hdr_len;
    pci_header_common_t *hdr = (pci_header_common_t*)self->mp_data;
    switch ((hdr->header_type) & 0b11)
    {
    case 0x0:
        hdr_len = sizeof(pci_header_type0_t);
        break;
    case 0x1:
        hdr_len = sizeof(pci_header_type1_t);
        break;
    case 0x2:
        hdr_len = sizeof(pci_header_type2_t);
        break;
    default:
        panic("PCI: What header type is this lol?");
        break;
    }

    if (count > hdr_len)
        count = hdr_len;

    memcpy(buffer, hdr, count);
    return count;
}

static vfs_node_ops_t g_node_ops = (vfs_node_ops_t) {
    .read = read
};

void pci_list()
{
    acpi_mcfg_t *mcfg = (acpi_mcfg_t*)acpi_lookup("MCFG");
    if (mcfg == NULL)
    {
        log("PCI: MCFG table not found!");
        return;
    }

    // Create /sys/pci
    vfs_node_t *sys_dir = vfs_lookup("/sys");
    vfs_node_t *pci_dir = sys_dir->ops->create(sys_dir, VFS_NODE_DIR, "pci");

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

                    // Create FS entry.
                    char name[16];
                    sprintf(name, "%02X:%02X:%02X", pci_hdr->class, pci_hdr->subclass, pci_hdr->prog_if);
                    vfs_node_t *file = pci_dir->ops->create(pci_dir, VFS_NODE_FILE, name);
                    file->ops = &g_node_ops;
                    file->mp_data = pci_hdr;

                    log("PCI: %X %X %02X:%02X:%02X", pci_hdr->vendor_id, pci_hdr->device_id, pci_hdr->class, pci_hdr->subclass, pci_hdr->prog_if);
                }
    }

    log("PCI: Successfully listed devices.");
}
