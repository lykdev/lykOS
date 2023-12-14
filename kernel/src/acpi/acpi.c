#include "acpi.h"

#include <lib/utils.h>

struct rsdp
{
    char signature[8];
    u8 checksum;
    char oem_id[6];
    u8 revision;
    u32 rsdt_addr;
} __attribute__((packed));

struct xsdp
{
    struct rsdp rsdp;

    u32 length;
    u32 xsdt_addr;
    u8 ext_checksum;
    u8 rsv[3];
} __attribute__((packed));

struct rsdt
{
    struct acpi_std_hdr hdr;
    u32 ptr_to_sdt[];
} __attribute__((packed));

struct xsdt
{
    struct acpi_std_hdr hdr;
    u64 ptr_to_sdt[];
} __attribute__((packed));

static bool sysdesc_is_ext;

static struct rsdt *rsdt;
static struct xsdt *xsdt;

void acpi_init()
{
    ASSERT(rsdp_request.response &&
           rsdp_request.response->address != NULL);

    void *p = rsdp_request.response->address;

    if (((struct rsdp*)p)->revision == 0)
    {
        sysdesc_is_ext = false;
        log("RSDP addr: 0x%llx", p);

        rsdt = (struct rsdt*)(((struct rsdp*)p)->rsdt_addr + HHDM);
        log("RSDT addr: 0x%llx", rsdt);
    }        
    else 
    {
        sysdesc_is_ext = true;
        log("XSDP addr: 0x%llx", p);

        xsdt = (struct xsdt *)(((struct xsdp*)p)->xsdt_addr + HHDM);
        log("XSDT addr: 0x%llx", xsdt);
    }
}

struct acpi_std_hdr* acpi_sysdesc_lookup(char *signature)
{
    if (!sysdesc_is_ext)
    {
        u64 entries = (rsdt->hdr.length - sizeof(rsdt->hdr)) / 4;

        for (u64 i = 0; i < entries; i++)
        {
            struct acpi_std_hdr *hdr = (struct acpi_std_hdr*)(rsdt->ptr_to_sdt[i] + HHDM);
            if (memcmp(hdr->signature, signature, 4) == 0)
                return hdr;
        }
    }
    else
    {
        u64 entries = (xsdt->hdr.length - sizeof(xsdt->hdr)) / 8;

        for (u64 i = 0; i < entries; i++)
        {
            struct acpi_std_hdr *hdr = (struct acpi_std_hdr*)(xsdt->ptr_to_sdt[i] + HHDM);
            if (memcmp(hdr->signature, signature, 4) == 0)
                return hdr;
        }
    }

    return NULL;
}