#include "pci.h"

#include <dev/acpi/acpi.h>

#include <lib/log.h>
#include <lib/hhdm.h>
#include <lib/heap.h>

typedef struct
{
    u64 base_addr;
    u16 pci_seg_group_no;
    u8  start_bus_no;
    u8  end_bus_no;
    u32 _rsv;
} __attribute__((packed)) segment_t;

typedef struct
{
    acpi_sdt_hdr_t hdr;
    u64            _rsv;
    segment_t      segments[];
} __attribute__((packed)) mcfg_t;

list_t pci_unit_list = LIST_INIT;

static mcfg_t *mcfg;

static pci_common_hdr_t* pci_get_config_space(u8 seg, u8 bus, u8 dev, u8 func)
{
    return (pci_common_hdr_t*)(HHDM + mcfg->segments[seg].base_addr +
                              ((uptr)bus << 20 | (uptr)dev << 15 | (uptr)func << 12));
}

void register_unit(pci_common_hdr_t *hdr)
{
    pci_unit_t *pci_dev = (pci_unit_t*)malloc(sizeof(pci_unit_t));

    pci_dev->hdr = hdr;

    list_append(&pci_unit_list, &pci_dev->list_element);
}

void pci_init()
{
    mcfg = (mcfg_t*)acpi_lookup("MCFG");

    usize seg_count = (mcfg->hdr.length - sizeof(mcfg->hdr)) / sizeof(segment_t);

    for (usize seg = 0; seg < seg_count; seg++)
        for (usize bus = 0; bus < 256; bus++)
            for (usize dev = 0; dev < 32; dev++)
            {
                // Check if the dev exists.
                pci_common_hdr_t *hdr = pci_get_config_space(seg, bus, dev, 0);
                if (hdr->vendor_id == 0xFFFF)
                    continue;

                // The first function is valid, register the unit.
                register_unit(hdr);

                // If it's a multi-function device check remaining functions.
                if (((hdr->header_type >> 7) & 1) != 1)
                    for (usize func = 1; func < 8; func++)
                    {
                        hdr = pci_get_config_space(seg, bus, dev, func);
                        if (hdr->vendor_id != 0xFFFF)
                            register_unit(hdr);
                    }
            }

    FOREACH (n, pci_unit_list)
    {
        pci_unit_t *pci_unit = LIST_GET_CONTAINER(n, pci_unit_t, list_element);
        
        log("%x %x", pci_unit->hdr->vendor_id, pci_unit->hdr->device_id);
    }
}
