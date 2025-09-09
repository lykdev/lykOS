#include <sys/module.h>

#include <common/assert.h>
#include <common/log.h>
#include <dev/pci.h>
#include <fs/vfs.h>
#include <mm/heap.h>
#include <lib/string.h>

#include "ps2kb.h"

bool __module_probe()
{
    return true;
}

void __module_install()
{
    ps2kb_setup();

    return;
}

void __module_destroy()
{
    log("gg");
}
