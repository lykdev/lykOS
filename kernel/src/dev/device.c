#include "device.h"

#include <lib/string.h>
#include <mm/heap.h>

static list_t g_buses = LIST_INIT;
static spinlock_t g_slock = SPINLOCK_INIT;

void bus_init(bus_t *bus, const char *name)
{
    *bus = (bus_t) {
        .name = heap_alloc(strlen(name) + 1),
        .devices = LIST_INIT,
        .drivers = LIST_INIT,
        .slock = SPINLOCK_INIT,
        .list_node = LIST_NODE_INIT
    };
    strcpy((char *)bus->name, name);
}

void bus_register(bus_t *bus)
{
    spinlock_acquire(&g_slock);

    list_append(&g_buses, &bus->list_node);

    spinlock_release(&g_slock);
}

bus_t *bus_lookup(const char *name)
{
    spinlock_acquire(&g_slock);

    FOREACH(n, g_buses)
    {
        bus_t *bus = LIST_GET_CONTAINER(n, bus_t, list_node);
        if (strcmp(bus->name, name) == 0)
        {
            spinlock_release(&g_slock);
            return bus;
        }
    }

    spinlock_release(&g_slock);
    return NULL;
}

void bus_register_device(bus_t *bus, device_t *dev)
{
    spinlock_acquire(&bus->slock);

    list_append(&bus->devices, &dev->list_node);
    if (dev->driver == NULL)
        FOREACH(n, bus->drivers)
        {
            driver_t *drv = LIST_GET_CONTAINER(n, driver_t, list_node);
            if (!drv->probe_device ||drv->probe_device(dev))
                break;
        }

    spinlock_release(&bus->slock);
}

void bus_register_driver(bus_t *bus, driver_t *drv)
{
    spinlock_acquire(&bus->slock);

    list_append(&bus->drivers, &drv->list_node);
    FOREACH(n, bus->devices)
    {
        device_t *dev = LIST_GET_CONTAINER(n, device_t, list_node);
        if (dev->driver != NULL)
            continue;
        if (!drv->probe_device || drv->probe_device(dev))
            break;
    }

    spinlock_release(&bus->slock);
}
