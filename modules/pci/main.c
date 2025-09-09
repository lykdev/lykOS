#include <common/hhdm.h>
#include <common/log.h>
#include <mm/heap.h>
#include <lib/printf.h>
#include <lib/string.h>

#include <dev/acpi/acpi.h>
#include <dev/devman.h>
#include <dev/pci.h>

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

bool __module_probe()
{
    return acpi_lookup("MCFG") != NULL;
}

void __module_install()
{
    bus_type_t *bus_type_pci = heap_alloc(sizeof(bus_type_t));
    *bus_type_pci = (bus_type_t) {
        .name = heap_alloc(4),
        .devices = LIST_INIT,
        .drivers = LIST_INIT,
        .slock = SPINLOCK_INIT,
        .list_node = LIST_NODE_INIT
    };
    strcpy((char *)bus_type_pci->name, "PCI");
    devman_reg_bus_type(bus_type_pci);

    acpi_mcfg_t *mcfg = (acpi_mcfg_t*)acpi_lookup("MCFG");

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

            device_t *dev = heap_alloc(sizeof(device_t));
            *dev = (device_t) {
                .name = heap_alloc(32),
                .data = pci_hdr,
                .driver = NULL,
                .driver_data = NULL,
                .list_node = LIST_NODE_INIT
            };
            sprintf((char *)dev->name, "%04X:%04X-%02X:%02X:%02X",
                pci_hdr->vendor_id, pci_hdr->device_id,
                pci_hdr->class, pci_hdr->subclass, pci_hdr->prog_if
            );
            devman_reg_device(bus_type_pci, dev);
        }
    }

    log("PCI: Successfully listed devices.");
}

void __module_destroy()
{

}
