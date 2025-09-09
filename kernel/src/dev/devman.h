#pragma once

#include <common/sync/spinlock.h>
#include <lib/list.h>

typedef struct device device_t;
typedef struct driver driver_t;
typedef struct bus_type bus_type_t;

struct device
{
    const char *name;

    void *data;

    struct driver *driver;
    void *driver_data;

    list_node_t list_node;
};

struct driver
{
    const char *name;

    void *match_data;

    bool (*probe)  (device_t *dev);
    void (*setup)  (device_t *dev);
    void (*remove) (device_t *dev);
    void (*suspend)(device_t *dev);
    void (*resume) (device_t *dev);

    list_node_t list_node;
};

struct bus_type
{
    const char *name;

    list_t devices;
    list_t drivers;

    spinlock_t slock;
    list_node_t list_node;
};

void devman_reg_bus_type(bus_type_t *bus);

bus_type_t *devman_get_bus_type(const char *name);

void devman_reg_device(bus_type_t *bus, device_t *dev);

void devman_reg_driver(bus_type_t *bus, driver_t *drv);

void devman_init();
