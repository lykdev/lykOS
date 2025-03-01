#include "resource.h"

#include <common/assert.h>
#include <common/log.h>
#include <mm/kmem.h>

resource_table_t resource_table_new() { return (resource_table_t){.resources = NULL, .length = 0, .lock = SLOCK_INIT}; }

void resource_table_expand(resource_table_t *table, uint amount)
{
    slock_acquire(&table->lock);

    table->resources = kmem_realloc(table, table->length, table->length + amount);
    table->length += amount;

    slock_release(&table->lock);
}

resource_t *resource_create_at(resource_table_t *table, int id, vfs_node_t *node, size_t offset, u8 flags)
{
    ASSERT(id < table->length);

    slock_acquire(&table->lock);

    resource_t *res = kmem_alloc(sizeof(resource_t));
    *res = (resource_t){
        .node = node,
        .offset = offset,
        .flags = flags,
    };
    table->resources[id] = res;

    slock_release(&table->lock);
}

resource_t *resource_get(resource_table_t *table, int id)
{
    slock_acquire(&table->lock);

    if (id < 0 || id >= table->length)
        return NULL;
    else
        return table->resources[id];

    slock_release(&table->lock);
}
