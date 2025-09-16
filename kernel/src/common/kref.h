#pragma once

#include <lib/def.h>

typedef u64 kref_t;

static inline void kref_init(kref_t *k, u64 val)
{
    *k = val;
}

static inline void kref_ref(kref_t *k)
{
    __sync_fetch_and_add(k, 1);
}

static inline bool kref_rel(kref_t *k)
{
    return __sync_sub_and_fetch(k, 1) == 0;
}
