#include "devman.h"

#include <common/panic.h>
#include <mm/heap.h>
#include <lib/def.h>
#include <lib/string.h>

#include <common/log.h>

static list_t g_buses = LIST_INIT;
static spinlock_t slock = SPINLOCK_INIT;

void devman_reg_bus_type(bus_type_t *bus)
{
    spinlock_acquire(&slock);

    list_append(&g_buses, &bus->list_node);

    spinlock_release(&slock);
}

bus_type_t *devman_get_bus_type(const char *name)
{
    spinlock_acquire(&slock);

    FOREACH(n, g_buses)
    {
        bus_type_t *bus = LIST_GET_CONTAINER(n, bus_type_t, list_node);
        if (strcmp(bus->name, name) == 0)
        {
            spinlock_release(&slock);
            return bus;
        }
    }

    spinlock_release(&slock);
    return NULL;
}

void devman_reg_device(bus_type_t *bus, device_t *dev)
{
    spinlock_acquire(&bus->slock);

    list_append(&bus->devices, &dev->list_node);
    if (dev->driver == NULL)
        FOREACH(n, bus->drivers)
        {
            driver_t *drv = LIST_GET_CONTAINER(n, driver_t, list_node);
            if (!drv->probe ||drv->probe(dev))
                break;
        }

    spinlock_release(&bus->slock);
}

void devman_reg_driver(bus_type_t *bus, driver_t *drv)
{
    spinlock_acquire(&bus->slock);

    list_append(&bus->drivers, &drv->list_node);
    FOREACH(n, bus->devices)
    {
        device_t *dev = LIST_GET_CONTAINER(n, device_t, list_node);
        if (dev->driver != NULL)
            continue;
        if (!drv->probe || drv->probe(dev))
            break;
    }

    spinlock_release(&bus->slock);
}

void devman_init()
{
    bus_type_t *bus_type_other = heap_alloc(sizeof(bus_type_t));
    *bus_type_other = (bus_type_t) {
        .name = heap_alloc(8),
        .devices = LIST_INIT,
        .drivers = LIST_INIT,
        .slock = SPINLOCK_INIT,
        .list_node = LIST_NODE_INIT
    };
    strcpy((char *)bus_type_other->name, "OTHER");
    devman_reg_bus_type(bus_type_other);
}
