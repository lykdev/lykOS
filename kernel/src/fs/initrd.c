#include "initrd.h"

#include <common/assert.h>
#include <common/limine/requests.h>
#include <common/log.h>
#include <common/panic.h>
#include <fs/vfs.h>
#include <fs/ustar.h>
#include <lib/def.h>
#include <lib/list.h>
#include <lib/path.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <mm/heap.h>

typedef struct
{
    vnode_t vn;

    list_t children;
    void *data;

    list_node_t list_node;
}
initrd_node_t;

static int open(vnode_t *self, const char *name, vnode_t **out)
{
    ASSERT(name && out);

    if (self->type != VNODE_DIR)
    {
        *out = NULL;
        return ENOTDIR;
    }

    initrd_node_t *parent = (initrd_node_t *)self;
    FOREACH(n, parent->children)
    {
        initrd_node_t *child = LIST_GET_CONTAINER(n, initrd_node_t, list_node);
        if (strcmp(child->vn.name, name) == 0)
        {
            VN_HOLD(&child->vn);
            *out = &child->vn;
            return EOK;
        }
    }

    *out = NULL;
    return ENOENT;
}

static int read(vnode_t *self, u64 offset, void *buffer, u64 size, u64 *out)
{
    initrd_node_t *node = (initrd_node_t *)self;
    uint file_content_size = node->vn.size;

    if (offset >= file_content_size)
    {
        *out = 0;
        return EOK;
    }

    if (offset + size >= file_content_size)
        size = file_content_size - offset;

    u8 *file_content = (u8 *)((uptr)node->data + offset);

    uint i;
    for (i = 0; i < size; i++)
        *(u8 *)buffer++ = *file_content++;

    *out = i;
    return EOK;
}

static int list(vnode_t *self, u64 *hint, const char **out)
{
    ASSERT(hint && out);

    if (self->type != VNODE_DIR)
    {
        *out = NULL;
        return ENOTDIR;
    }

    initrd_node_t *parent = (initrd_node_t *)self;

    if (*hint == 0xFFFF)
    {
        *out = NULL;
        return EOK;
    }

    list_node_t *next;
    if (*hint == 0)
        next = parent->children.head;
    else
        next = ((list_node_t *)*hint)->next;

    if (next)
    {
        *hint = (u64)next;

        initrd_node_t *child = LIST_GET_CONTAINER(next, initrd_node_t, list_node);
        *out = (const char *)&child->vn.name;
        return EOK;
    }
    else
    {
        *hint = 0xFFFF;
        *out = NULL;
        return EOK;
    }
}

static vnode_ops_t ustar_file_ops = (vnode_ops_t) {
    .read = read,
    .open = open,
    .list = list
};

static initrd_node_t initrd_root_node = {
    .vn = (vnode_t) {
        .name = "/",
        .type = VNODE_DIR,
        .ops = &ustar_file_ops
    },
    .children = LIST_INIT,
    .data = NULL,
    .list_node = LIST_NODE_INIT
};

bool initrd_probe(block_device_t *blk_dev [[maybe_unused]])
{
    return true;
}

bool initrd_get_root_vnode(block_device_t *blk_dev [[maybe_unused]], vnode_t **out)
{
    *out = &initrd_root_node.vn;
    return true;
}

static filesystem_type_t initrd_fs = {
    .name = "initrd",
    .probe = initrd_probe,
    .get_root_vnode = initrd_get_root_vnode
};

static void process_entry(ustar_hdr_t *hdr)
{
    path_normalize(hdr->filename, hdr->filename);

    // INODE
    initrd_node_t *node = heap_alloc(sizeof(initrd_node_t));
    *node = (initrd_node_t) {
        .children = LIST_INIT,
        .data = (void*)((uptr)hdr + 512),
        .list_node = LIST_NODE_INIT
    };
    // VNODE
    char *p = strrchr(hdr->filename, '/');
    if (p)
        strcpy(node->vn.name, p + 1);
    else
        strcpy(node->vn.name, hdr->filename);
    switch (hdr->type)
    {
        case USTAR_REG:   node->vn.type = VNODE_REG;   break;
        case USTAR_CHAR:  node->vn.type = VNODE_CHAR;  break;
        case USTAR_BLOCK: node->vn.type = VNODE_BLOCK; break;
        case USTAR_DIR:   node->vn.type = VNODE_DIR;   break;
        case USTAR_FIFO:  node->vn.type = VNODE_FIFO;  break;
        default:
            panic("Invalid file type inside initrd!");
        break;
    }
    node->vn.size = ustar_read_field(hdr->size, 12);
    node->vn.ops = &ustar_file_ops;
    node->vn.slock = SPINLOCK_INIT;
    node->vn.ref_count = 0;

    vnode_t *curr = &initrd_root_node.vn;
    const char *path = hdr->filename;
    while (*path)
    {
        char *slash = strchr(path, '/');
        if (slash)
            *slash = '\0';

        vnode_t *next;
        if (curr->ops->open(curr, path, &next) == EOK)
            curr = next;
        else
            break;

        path += strlen(path) + 1;
    }

    initrd_node_t *parent = (initrd_node_t *)curr;
    list_append(&parent->children, &node->list_node);
}

void initrd_init()
{
    ustar_hdr_t *hdr = NULL;
    for (uint i = 0; i < request_module.response->module_count; i++)
        if (strcmp(request_module.response->modules[i]->path, "/initrd.tar") == 0)
        {
            hdr = (ustar_hdr_t *)request_module.response->modules[i]->address;
            break;
        }
    if (hdr == NULL)
        panic("Initrd module not found!");

    while (true)
    {
        if (hdr->magic[0] == '\0' && ((ustar_hdr_t *)((uptr)hdr + 512))->magic[0] == '\0')
            break;

        if (strcmp(hdr->magic, USTAR_MAGIC) != 0)
            panic("Invalid USTAR entry!");

        process_entry(hdr);

        uint file_size = ustar_read_field(hdr->size, 12);
        uint blocks = (file_size + 512 - 1) / 512;

        hdr = (ustar_hdr_t *)((uptr)hdr + (blocks + 1) * 512);
    }

    vfs_mount(NULL, &initrd_fs, "/");
}
