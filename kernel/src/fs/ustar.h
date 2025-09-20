#pragma once

#include <lib/def.h>

#define USTAR_MAGIC "ustar"

#define USTAR_REG          '0'
#define USTAR_HARDLINK     '1'
#define USTAR_SYMLINK      '2'
#define USTAR_CHAR         '3'
#define USTAR_BLOCK        '4'
#define USTAR_DIR          '5'
#define USTAR_FIFO         '6'
#define USTAR_CONTIG       '7'

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
}
__attribute__((packed))
ustar_hdr_t;

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
