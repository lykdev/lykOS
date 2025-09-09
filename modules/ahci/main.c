#include <sys/module.h>

#include <common/assert.h>
#include <common/log.h>
#include <dev/pci.h>
#include <dev/devman.h>
#include <fs/vfs.h>
#include <mm/heap.h>
#include <lib/string.h>

#include "ahci.h"

bool __module_probe()
{
    return true;
}

void __module_install()
{
    driver_t *drv = heap_alloc(sizeof(driver_t));
    *drv = (driver_t) {
        .name = heap_alloc(8),
        .match_data = NULL,
        .list_node = LIST_NODE_INIT
    };
    strcpy((char *)drv->name, "AHCI");
    devman_reg_driver(devman_get_bus_type("PCI"), drv);
}

void __module_destroy()
{
    log("gg");
}

MODULE_DEP("PCI")

MODULE_NAME("AHCI")
MODULE_VERSION("0.1")
MODULE_AUTHOR("Matei Lupu")
