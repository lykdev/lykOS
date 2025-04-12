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
#include <mm/kmem.h>

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
    vfs_node_t vfs_node;

    list_node_t list_elem;
}
initrd_entry_t;
static list_t g_entry_list;

static u64 read(vfs_node_t *self, u64 offset, void *buffer, u64 size)
{
    if (self->type != VFS_NODE_FILE)
        return -1;

    initrd_entry_t *entry = self->mp_node;
    uint file_content_size = ustar_read_field(entry->ustar_data->size, 12);

    if (offset >= file_content_size)
        return -1;


    if (offset + size >= file_content_size)
        size = file_content_size - offset;

    u8 *file_content = (u8 *)((uptr)entry->ustar_data + 512 + offset);

    uint i;
    for (i = 0; i < size; i++)
        *(u8 *)buffer++ = *file_content++;

    return i;
}

static vfs_node_t *lookup(vfs_node_t *self, const char *name)
{
    if (self->type != VFS_NODE_DIR)
        return NULL;

    // Maybe optimise this later... if you care enough.
    char path[100] = "";
    strcat(path, ((initrd_entry_t *)self->mp_node)->ustar_data->filename);
    strcat(path, "/");
    strcat(path, name);

    FOREACH(n, g_entry_list)
    {
        initrd_entry_t *node = LIST_GET_CONTAINER(n, initrd_entry_t, list_elem);
        if (strcmp(path, node->ustar_data->filename) == 0)
            return &node->vfs_node;
    }

    return NULL;
}

const char *list(vfs_node_t *self, uint *index)
{
    if (self->type != VFS_NODE_DIR)
        return NULL;

    // Maybe optimise this later... if you care enough.
    char path[100] = "";
    strcat(path, ((initrd_entry_t *)self->mp_node)->ustar_data->filename);
    strcat(path, "/");

    uint l_index = 0;
    FOREACH(n, g_entry_list)
    {
        initrd_entry_t *node = LIST_GET_CONTAINER(n, initrd_entry_t, list_elem);
        if (strncmp(path, node->ustar_data->filename, strlen(path)) == 0)
        {
            if (l_index == *index)
            {
                (*index)++;
                return (const char *)&node->ustar_data->filename[strlen(path)];
            }
            else
                l_index++;
        }
    }

    return NULL;
}

static vfs_node_ops_t g_node_ops = (vfs_node_ops_t) {
    .read = read,
    .write = NULL,
    .lookup = lookup,
    .list = list
};

static vfs_mountpoint_t g_mountpoint;

static void process_entry(ustar_hdr_t *hdr)
{
    path_normalize(hdr->filename, hdr->filename);

    initrd_entry_t *node = kmem_alloc(sizeof(initrd_entry_t));
    node->ustar_data = hdr;

    node->vfs_node = (vfs_node_t) {
        .type = hdr->type == '5' ? VFS_NODE_DIR : VFS_NODE_FILE,
        .mp_node = node,
        .ops = &g_node_ops
    };

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
    vfs_mount("initrd", &g_mountpoint);

    log("Mounted initrd at `/initrd`.");
}
