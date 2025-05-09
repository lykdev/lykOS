#include "acpi.h"

#include <common/assert.h>
#include <common/hhdm.h>
#include <common/limine/requests.h>
#include <common/log.h>
#include <lib/string.h>

typedef struct
{
    char signature[8];
    u8   checksum;
    char oemid[6];
    u8   revision;
    u32  rsdt_addr;

    u32 length;
    u64 xsdt_addr;
    u8  ext_checksum;
    u8  _rsv[3];
}
__attribute__((packed))
acpi_sdp_t;

static acpi_sdt_t *root_sdt;
static bool extended;

void acpi_init()
{
    if (request_rsdp.response->address == 0)
    {
        log("ACPI: No RSDP found");
        return;
    }

    acpi_sdp_t *sdp = (acpi_sdp_t*)request_rsdp.response->address;
    if (sdp->revision < 2)
        root_sdt = (acpi_sdt_t*)((u64)sdp->rsdt_addr + HHDM), extended = false;
    else
        root_sdt = (acpi_sdt_t*)((u64)sdp->xsdt_addr + HHDM), extended = true;

    log("ACPI: Found valid root SDT.");
}

acpi_sdt_t *acpi_lookup(const char *signature)
{
    ASSERT_C(root_sdt, "Tried using ACPI before initialization of it.");

    int entries = (root_sdt->length - sizeof(acpi_sdt_t)) / (extended ? 8 : 4);
    u32 *pointers32 = (u32*)((uptr)root_sdt + sizeof(acpi_sdt_t));
    u64 *pointers64 = (u64*)((uptr)root_sdt + sizeof(acpi_sdt_t));

    for (int i = 0; i < entries; i++)
    {
        acpi_sdt_t *sdt = (acpi_sdt_t*)((uptr)(extended ? pointers64[i] : pointers32[i]) + HHDM);
        if (strncmp(sdt->signature, signature, 4) == 0)
            return sdt;
    }

    return NULL;
}
