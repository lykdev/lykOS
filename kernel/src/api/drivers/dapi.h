#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

typedef struct dapi_device dapi_device_t;
typedef struct dapi_driver dapi_driver_t;
typedef struct dapi_res    dapi_res_t;

enum dapi_dev_type_t
{
    DAPI_AUDIO,
    DAPI_IO,
    DAPI_NETWORK,
    DAPI_STORAGE
};

struct dapi_device
{
    dapi_dev_type_t type;
    char *name;
    dapi_driver *driver;
};

struct dapi_driver
{
    char *name;
    char *author;
    void (*setup)();
    void (*init_device)(dapi_device_t *device);
    void (*remove_device)(dapi_device_t *device);
    void (*destroy)();
};

enum dapi_res_type_t
{
    DAPI_IRQ,
    DAPI_IOPORT,
    DAPI_MEMORY
};

struct dapi_res
{
    dapi_res_type_t type;
    uint64_t start, count;
};
