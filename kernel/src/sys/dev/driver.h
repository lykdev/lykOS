#pragma once

#include <lib/list.h>
#include <sys/elf.h>

typedef struct
{
    elf_object_t *elf_obj;
    list_node_t list_elem;
} driver_t;

extern list_t g_driver_list;
