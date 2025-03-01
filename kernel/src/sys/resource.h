#pragma once

#include <common/slock.h>
#include <fs/vfs.h>

typedef enum
{
    RESOURCE_READ = 1 << 0,
    RESOURCE_WRITE = 1 << 1
} resource_mode_t;

typedef struct
{
    vfs_node_t *node;
    u64 offset;
    u8 flags;
} resource_t;

typedef struct
{
    resource_t **resources;
    int length;
    slock_t lock;
} resource_table_t;

resource_table_t resource_table_new();

void resource_table_expand(resource_table_t *table, uint amount);

resource_t *resource_create_at(resource_table_t *table, int id, vfs_node_t *node, size_t offset, u8 flags);

resource_t *resource_get(resource_table_t *table, int id);
