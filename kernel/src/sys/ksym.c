#include "ksym.h"

#include <common/limine/requests.h>
#include <common/panic.h>
#include <lib/string.h>

typedef struct
{
    uint64_t addr;
    char name[];
}
__attribute__((packed))
ksym_t;

static void *ksym_data = NULL;
static u64   ksym_size = 0;

uptr ksym_resolve_symbol(const char *name)
{
    void *p = ksym_data;
    while (p < ksym_data + ksym_size)
    {
        ksym_t *sym = (ksym_t*)p;

        if (strcmp(name, (const char *)&sym->name) == 0)
            return sym->addr;
        
        p += sizeof(sym->addr) + strlen(sym->name) + 1;
    }

    return 0;
}

void ksym_load_symbols()
{
    for (uint i = 0; i < request_module.response->module_count; i++)
    if (strcmp(request_module.response->modules[i]->path, "/kernel_symbols.bin") == 0)
    {
        ksym_data = (void*)request_module.response->modules[i]->address;
        ksym_size = request_module.response->modules[i]->size;
        break;
    }

    if (ksym_data == NULL)
        panic("\"kernel_symbols.bin\" bootloader module not found!");
}