#include <common/assert.h>
#include <common/log.h>
#include <dev/pci.h>
#include <fs/vfs.h>
#include <mm/heap.h>
#include <lib/string.h>

#include "ahci.h"

bool __module_probe()
{
    vfs_node_t *dir = vfs_lookup("/sys/pci");
    if (dir == NULL)
        panic("/sys/pci not found!");

    u64 idx = 0;
    const char *name;
    while ((name = dir->dir_ops->list(dir, &idx)))
        // Mass Storage Controller - Serial ATA Controller - AHCI
        if (strcmp(name, "01:06:01"))
            return true;
        else
            continue;

    return false;
}

void __module_install()
{
    pci_header_type0_t pci_hdr;
    vfs_node_t *file = vfs_lookup("/sys/pci/01:06:01");
    file->file_ops->read(file, 0, &pci_hdr, sizeof(pci_header_type0_t));

    ahci_setup(pci_hdr.bar[5]);

    return;
}

void __module_destroy()
{
    log("gg");
}
