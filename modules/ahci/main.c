#include <sys/module.h>

#include <common/assert.h>
#include <common/log.h>
#include <dev/pci.h>
#include <dev/device.h>
#include <dev/storage.h>
#include <fs/vfs.h>
#include <mm/heap.h>
#include <lib/string.h>

#include "ahci.h"

bool ahci_probe_device(device_t *dev)
{
    pci_header_common_t *pci_hdr = dev->hardware_data;
    if (!pci_hdr)
        return false;

    if (pci_hdr->class == 1 && pci_hdr->subclass == 6 && pci_hdr->prog_if == 1)
        return true;

    return false;
}

void ahci_setup_device(device_t *dev)
{
    ahci_setup(((pci_header_type0_t *)dev->hardware_data)->bar[5]);
}

void ahci_remove_device(device_t *dev)
{

}

driver_t ahci_driver = (driver_t) {
    .name = "ahci",
    .list_node = LIST_NODE_INIT,
    .probe_device  = &ahci_probe_device,
    .setup_device  = &ahci_setup_device,
    .remove_device = &ahci_remove_device
};

void __module_install()
{;
    bus_t *pci_bus = bus_lookup("pci");

    if (pci_bus)
        bus_register_driver(pci_bus, &ahci_driver);
    else
        log("Failed to locate `pci` bus type!");
}

void __module_destroy()
{
    log("gg");
}

MODULE_DEP("PCI")

MODULE_NAME("AHCI")
MODULE_VERSION("0.1")
MODULE_AUTHOR("Matei Lupu")
