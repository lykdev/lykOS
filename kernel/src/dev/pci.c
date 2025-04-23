#include "pci.h"

#include <common/hhdm.h>
#include <common/log.h>
#include <dev/acpi/acpi.h>

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

void pci_list()
{
    acpi_mcfg_t *mcfg = (acpi_mcfg_t*)acpi_lookup("MCFG");
    if (mcfg == NULL)
    {
        log("PCI: MCFG table not found!");
        return;
    }

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

                    log("PCI: %x %x %x %x %x", pci_hdr->vendor_id, pci_hdr->device_id, pci_hdr->class, pci_hdr->subclass, pci_hdr->prog_if);
                }
    }

    log("PCI: Successfully listed devices.");
}
