#pragma once

#include <lib/def.h>

#define AT_NULL     0
#define AT_IGNORE   1
#define AT_EXECFD   2
#define AT_PHDR     3
#define AT_PHENT    4
#define AT_PHNUM    5
#define AT_PAGESZ   6
#define AT_BASE     7
#define AT_FLAGS    8
#define AT_ENTRY    9
#define AT_NOTELF  10
#define AT_UID     11
#define AT_EUID    12
#define AT_GID     13
#define AT_EGID    14

#define X86_64_AUXV_SECURE 23

typedef struct {
    uint64_t entry;
    uint64_t phdr;
    uint64_t phent;
    uint64_t phnum;
}
x86_64_auxv_t;
