#include "initrd.h"

#include <common/assert.h>
#include <common/limine/requests.h>
#include <common/log.h>
#include <common/panic.h>
#include <fs/vfs.h>
#include <lib/def.h>
#include <lib/list.h>
#include <lib/path.h>
#include <lib/string.h>
#include <mm/heap.h>

// USTAR

#define USTAR_MAGIC "ustar"

typedef struct
{
    char filename[100];
    char filemode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char type;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char filename_prefix[155];
    char _rsv[12];
} __attribute__((packed)) ustar_hdr_t;

static u64 ustar_read_field(const char *str, u64 size)
{
    u64 n = 0;
    const char *c = str;
    while (size-- > 0 && *c != '\0')
    {
        n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}

typedef struct
{
    ustar_hdr_t *ustar_data;
    vnode_t vfs_node;

    list_node_t list_elem;
}
initrd_entry_t;
static list_t g_entry_list;

static int read(vnode_t *self, u64 offset, void *buffer, u64 size, u64 *out)
{
    initrd_entry_t *entry = self->mp_data;
    uint file_content_size = ustar_read_field(entry->ustar_data->size, 12);

    if (offset >= file_content_size)
    {
        *out = 0;
        return EOK;
    }

    if (offset + size >= file_content_size)
        size = file_content_size - offset;

    u8 *file_content = (u8 *)((uptr)entry->ustar_data + 512 + offset);

    uint i;
    for (i = 0; i < size; i++)
        *(u8 *)buffer++ = *file_content++;

    *out = i;
    return EOK;
}

static int lookup(vnode_t *self, const char *name, vnode_t **out)
{
    // Maybe optimise this later... if you care enough.
    char path[100] = "";
    strcat(path, ((initrd_entry_t *)self->mp_data)->ustar_data->filename);
    strcat(path, "/");
    strcat(path, name);

    FOREACH(n, g_entry_list)
    {
        initrd_entry_t *node = LIST_GET_CONTAINER(n, initrd_entry_t, list_elem);
        if (strcmp(path, node->ustar_data->filename) == 0)
        {
            *out = &node->vfs_node;
            return EOK;
        }
    }

    *out = NULL;
    return -ENOENT;
}

static int list(vnode_t *self, u64 *hint, const char **out)
{
    // Maybe optimise this later... if you care enough.
    char path[100] = "";
    strcat(path, ((initrd_entry_t *)self->mp_data)->ustar_data->filename);
    strcat(path, "/");

    uint l_index = 0;
    FOREACH(n, g_entry_list)
    {
        initrd_entry_t *node = LIST_GET_CONTAINER(n, initrd_entry_t, list_elem);
        if (strncmp(path, node->ustar_data->filename, strlen(path)) == 0)
        {
            if (l_index == *hint)
            {
                (*hint)++;
                *out = (const char *)&node->ustar_data->filename[strlen(path)];
                return EOK;
            }
            else
                l_index++;
        }
    }

    *out = NULL;
    return EOK;
}

static vnode_ops_t g_file_ops = (vnode_ops_t) {
    .read = read,
};

static vnode_ops_t g_dir_ops = (vnode_ops_t) {
    .lookup = lookup,
    .list = list
};

static vfs_mountpoint_t g_mountpoint;

static void process_entry(ustar_hdr_t *hdr)
{
    path_normalize(hdr->filename, hdr->filename);

    initrd_entry_t *node = heap_alloc(sizeof(initrd_entry_t));
    node->ustar_data = hdr;

    node->vfs_node = (vnode_t) {
        .type = hdr->type == '5' ? VFS_NODE_DIR : VFS_NODE_FILE,
        .mp_data = node,
        .ops = hdr->type == '5' ? (void*)&g_dir_ops : (void*)&g_file_ops
    };
    char *p = strrchr(hdr->filename, '/');
    if (p)
        strcpy(node->vfs_node.name, p + 1);
    else
        strcpy(node->vfs_node.name, hdr->filename);

    list_append(&g_entry_list, &node->list_elem);
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

    g_mountpoint.root_node = &LIST_GET_CONTAINER(g_entry_list.head, initrd_entry_t, list_elem)->vfs_node;
    vfs_mount("", &g_mountpoint);
}
