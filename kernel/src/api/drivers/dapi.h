#pragma once

typedef struct dapi_dev dapi_dev_t;
typedef struct dapi_drv dapi_drv_t;

enum dapi_dev_type_t
{
    DAPI_AUDIO,
    DAPI_IO,
    DAPI_NETWORK,
    DAPI_STORAGE
};

struct dapi_dev
{
    dapi_dev_type_t type;
    char *name;
    void *ops;
}
__attribute__((packed));

struct dapi_drv
{
    char *name;
    char *author;
    void *ops;
}
__attribute__((packed));

struct drv_ops
{
    bool (*setup)();
}
__attribute__((packed));
