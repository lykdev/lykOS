#include "acpi.h"

#include <lib/log.h>
#include <lib/assert.h>
#include <lib/hhdm.h>
#include <lib/str.h>

typedef struct
{
    char signature[8];
    u8   checksum;
    char oem_id[6];
    u8   revision;
    u32  rsdt_addr;

    u32  length;
    u64  xsdt_addr;
    u8   checksum_ext;
    u8   _rsv[3];
} __attribute__((packed)) xsdp_t;

typedef struct
{
    acpi_sdt_hdr_t hdr;
    u32 ptr_to_other_sdt[];
} __attribute__((packed)) rsdt_t;

typedef struct
{
    acpi_sdt_hdr_t hdr;
    u64 ptr_to_other_sdt[];
} __attribute__((packed)) xsdt_t;

static bool is_ext;
static rsdt_t *rsdt;
static xsdt_t *xsdt;

void acpi_init()
{
    ASSERT(req_rsdp.response != NULL
        || req_rsdp.response->address > 0);

    // Get XSDP
    xsdp_t *xsdp = (xsdp_t*)(req_rsdp.response->address);

    // Get RSDT/XSDT
    if (xsdp->revision == 0)
    {
        is_ext = false;
        rsdt = (rsdt_t*)(xsdp->rsdt_addr + HHDM);
    }
    else if (xsdp->revision == 2)
    {
        is_ext = true;
        xsdt = (xsdt_t*)(xsdp->xsdt_addr + HHDM);
    }        
    else
        panic("Invalid ACPI version found in the RSDP. %u", xsdp->revision);
}

acpi_sdt_hdr_t *acpi_lookup(const char *signature)
{
    usize len = xsdt->hdr.length - sizeof(xsdt->hdr);
    usize entries = is_ext ? len / 8 : len / 4;

    for (usize i = 0; i < entries; i++)
    {
        acpi_sdt_hdr_t *sdt;

        if (is_ext)
            sdt = (acpi_sdt_hdr_t*)(xsdt->ptr_to_other_sdt[i] + HHDM);
        else
            sdt = (acpi_sdt_hdr_t*)(rsdt->ptr_to_other_sdt[i] + HHDM);

        if (!strncmp(sdt->signature, signature, 4))
            return sdt; 
    }

    return NULL;
}
