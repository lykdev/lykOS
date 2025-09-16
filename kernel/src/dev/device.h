#pragma once

#include <common/sync/spinlock.h>
#include <lib/list.h>

typedef struct bus    bus_t;
typedef struct device device_t;
typedef struct driver driver_t;

struct bus
{
    const char *name;

    list_t devices;
    list_t drivers;

    list_node_t list_node;
    spinlock_t slock;
};

struct device
{
    const char *name;

    void *hardware_data;

    driver_t *driver;
    void *driver_data;

    list_t children;
    list_node_t list_node;
};

struct driver
{
    const char *name;

    bool (*probe_device)  (device_t *dev);
    void (*setup_device)  (device_t *dev);
    void (*remove_device) (device_t *dev);
    void (*suspend_device)(device_t *dev);
    void (*resume_device) (device_t *dev);

    list_node_t list_node;
};

void bus_init(bus_t *bus, const char *name);

void bus_register(bus_t *bus);

bus_t *bus_lookup(const char *name);

void bus_register_device(bus_t *bus, device_t *dev);

void bus_register_driver(bus_t *bus, driver_t *drv);
