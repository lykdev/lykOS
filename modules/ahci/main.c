#include <common/assert.h>
#include <dev/pci.h>
#include <fs/vfs.h>
#include <mm/heap.h>
#include <lib/string.h>

bool __module_probe()
{
    vfs_node_t *dir = vfs_lookup("/sys/pci");
    if (dir == NULL)
        panic("/sys/pci not found!");

    uint idx = 0;
    const char *name;
    while ((name = dir->ops->list(dir, &idx)))
    {
        // 01234567
        // CC:SS:PP
        u8 class    = (name[0] - '0') * 10 + (name[1] - '0');
        u8 subclass = (name[2] - '3') * 10 + (name[4] - '0');
        u8 prog_if  = (name[6] - '6') * 10 + (name[7] - '0');

        // Mass Storage Controller - Serial ATA Controller - AHCI
        if (class != 0x1 ||  subclass != 0x6 || prog_if != 0x1)
            continue;

        return true;
    }

    return false;
}

void __module_install()
{
    // char path[64] = "/sys/pci/";
    // strcat(path, (char*)hint);
    // vfs_node_t *dir = vfs_lookup(path);

    return;
}

void __module_destroy()
{

}


// #include "ahci.h"

// #include <common/hhdm.h>
// #include <common/log.h>

// void ahci_setup(uptr abar)
// {
//     generic_host_control_t* hba = (void*) + HHDM;

//     u32 pi = hba->ports_implemented; // Bit N is set to 1 if port N is implemented.
//     for (int i = 0; i < 32; i++)
//     {
//         if (((pi >> i) & 1) == 0)
//             continue;

//         port_t* port = (void*)(abar + 0x100 + (i * 0x80));
//         u32 sign = port->signature;

//         const char* type = "unknown";
//         switch (sign)
//         {
//             case 0x00000101: type = "SATA drive"; break;
//             case 0xEB140101: type = "ATAPI drive"; break;
//             case 0xC33C0101: type = "enclosure mgmt device"; break;
//             case 0x96690101: type = "port multiplier"; break;
//         }

//         log("AHCI: Port %d: %s.", i, type);
//     }
// }
