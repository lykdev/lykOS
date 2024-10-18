#include "initrd.h"

#include <utils/def.h>
#include <utils/limine/requests.h>
#include <utils/log.h>
#include <utils/string.h>

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

static u64 read_field(const char *str, u64 size)
{
    u64 n = 0;
    const unsigned char *c = str;
    while (size-- > 0)
    {
        n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}

//

ustar_hdr_t *root = NULL;

void initrd_init()
{
    if (request_module.response == NULL
    ||  request_module.response->module_count == 0)
    {
        log("Initrd module not found!");
        return;
    }

    root = (ustar_hdr_t*)request_module.response->modules[0]->address;
    if (strcmp(root->magic, USTAR_MAGIC) != 0)
    {
        log("Initrd module is invalid!");
        return;
    }

    log("Initrd ramdisk loaded.");
}
