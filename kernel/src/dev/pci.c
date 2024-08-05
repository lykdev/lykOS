#include "pci.h"

#include <dev/acpi/acpi.h>

#include <lib/log.h>
#include <lib/hhdm.h>

typedef struct
{
    u64 base_addr;
    u16 pci_seg_group_no;
    u8  start_bus_no;
    u8  end_bus_no;
    u32 _rsv;
} segment_t;

typedef struct
{
    acpi_sdt_hdr_t hdr;
    segment_t      segments[];
} mcfg_t;

void pci_init()
{
    mcfg_t *mcfg = (mcfg_t*)acpi_lookup("MCFG");

    usize seg_count = (mcfg->hdr.length - sizeof(mcfg->hdr)) / sizeof(segment_t);

    log("> %llu", seg_count);
}
