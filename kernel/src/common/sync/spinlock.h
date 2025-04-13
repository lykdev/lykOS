#pragma once

#include <arch/cpu.h>
#include <lib/def.h>

typedef struct
{
    u8 lock;
    bool prev_int_state;
}
spinlock_t;

#define SPINLOCK_INIT (0)

void spinlock_acquire(volatile spinlock_t *lock);

void spinlock_release(volatile spinlock_t *lock) __attribute__((always_inline));
