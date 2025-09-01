#include <common/assert.h>
#include <common/log.h>
#include <dev/pci.h>
#include <fs/vfs.h>
#include <mm/heap.h>
#include <lib/string.h>

#include "ahci.h"

bool __module_probe()
{
    vnode_t *dir;
    vfs_open("/sys/pci", &dir);
    if (dir == NULL)
        panic("/sys/pci not found!");

    u64 idx = 0;
    const char *name;
    while (dir->ops->list(dir, &idx, &name), name)
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
    vnode_t *file;
    vfs_open("/sys/pci/01:06:01", &file);
    u64 out;
    file->ops->read(file, 0, &pci_hdr, sizeof(pci_header_type0_t), &out);

    ahci_setup(pci_hdr.bar[5]);

    return;
}

void __module_destroy()
{
    log("gg");
}
