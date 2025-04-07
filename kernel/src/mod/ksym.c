#include "mod.h"

#include <common/limine/requests.h>
#include <common/log.h>
#include <common/panic.h>
#include <lib/string.h>

typedef struct
{
    uint64_t addr;
    char name[];
}
__attribute__((packed))
symbol_t;

static void *data;
static u64   size;

void mod_ksym_init()
{
    for (uint i = 0; i < request_module.response->module_count; i++)
        if (strcmp(request_module.response->modules[i]->path, "/kernel_symbols.bin") == 0)
        {
            data = (void*)request_module.response->modules[i]->address;
            size = request_module.response->modules[i]->size;
            break;
        }

    if (data == NULL)
        panic("Kernel symbols module not found!");
}

uptr mod_resolve_sym(const char *name)
{
    void *p = data;
    while (p < data + size)
    {
        symbol_t *sym = (symbol_t*)p;

        if (strcmp(name, (const char *)&sym->name) == 0)
            return sym->addr;
        
        p += sizeof(sym->addr) + strlen(sym->name) + 1;
    }

    return 0;
}