#include "initrd.h"

#include <core/fs/vfs.h>
#include <core/mm/kmem.h>

#include <utils/def.h>
#include <utils/list.h>
#include <utils/log.h>
#include <utils/path.h>
#include <utils/panic.h>
#include <utils/string.h>

#include <utils/limine/requests.h>

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
} __attribute__((packed))
ustar_hdr_t;

static u64 ustar_read_field(const char *str, u64 size)
{
    u64 n = 0;
    const u8 *c = str;
    while (size-- > 0)
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
    vfs_node_t  vfs_node;

    list_node_t list_elem;
}
initrd_entry_t;
static list_t g_entry_list;

static int read(vfs_node_t *self, u64 offset, u64 size, void *buffer)
{
    if (self->type != VFS_NODE_FILE)
        return -1;

    initrd_entry_t *entry = self->mp_node;
    uint file_content_size = ustar_read_field(entry->ustar_data->size, 12) - 512;

    if (offset >= file_content_size)
        return -1;

    if (offset + size >= file_content_size)
        size = file_content_size - offset;

    u8  *file_content = (u8*)((uptr)entry->ustar_data + 512 + offset);
    for (uint i = 0; i < size; i++)
        *(u8*)buffer++ = *file_content++;

    return size;
}

static int lookup(vfs_node_t *self, char *name, vfs_node_t **out)
{
    if (self->type != VFS_NODE_DIR)
    {
        *out = NULL;
        return -1;
    }
        
    // Maybe optimise this later... if you care enough.
    char path[100] = "";
    strcat(path, ((initrd_entry_t*)self->mp_node)->ustar_data->filename);
    strcat(path, "/");
    strcat(path, name);

    FOREACH(n, g_entry_list)
    {
        initrd_entry_t *node = LIST_GET_CONTAINER(n, initrd_entry_t, list_elem);
        if (strcmp(path, node->ustar_data->filename) == 0)
        {
            *out = &node->vfs_node;
            return 0;
        }
    }

    *out = NULL;
    return 0;
}

int list(vfs_node_t *self, uint *index, char **out)
{
    if (self->type != VFS_NODE_DIR)
    {
        *out = NULL;
        return -1;
    }

    // Maybe optimise this later... if you care enough.
    char path[100] = "";
    strcat(path, ((initrd_entry_t*)self->mp_node)->ustar_data->filename);
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
                *out = &node->ustar_data->filename;
                return 0;
            }
            else
                l_index++;    
        }
    }

    *out = NULL;
    return 0;
}

static vfs_node_ops_t g_node_ops = (vfs_node_ops_t) {
    .read = read,
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
    if (request_module.response == NULL
    ||  request_module.response->module_count == 0)
        panic("Initrd module not found!");

    ustar_hdr_t *hdr = (ustar_hdr_t*)request_module.response->modules[0]->address;
    while (hdr->magic[0] != '\0')
    {
        if (strcmp(hdr->magic, USTAR_MAGIC) != 0)
            panic("Invalid USTAR entry!");

        process_entry(hdr);

        uint file_size = ustar_read_field(hdr->size, 12);
        size_t blocks = (file_size + 512 - 1) / 512; 
        hdr = (ustar_hdr_t *)((uptr)hdr + 512 + blocks * 512);
    }

    g_mountpoint.root_node = &LIST_GET_CONTAINER(g_entry_list.head, initrd_entry_t, list_elem)->vfs_node;
    vfs_mount("initrd", &g_mountpoint);

    log("Initrd loaded.");
}

