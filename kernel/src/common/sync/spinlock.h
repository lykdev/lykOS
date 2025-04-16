#pragma once

#include <arch/cpu.h>
#include <lib/def.h>

typedef struct
{
    u8 lock;
    bool prev_int_state;
}
spinlock_t;

#define SPINLOCK_INIT ((spinlock_t) {.lock = 0, .prev_int_state = 0 })

void spinlock_acquire(volatile spinlock_t *slock);

void spinlock_release(volatile spinlock_t *slock) __attribute__((always_inline));

void spinlock_primitive_acquire(volatile spinlock_t *slock);

void spinlock_primitive_release(volatile spinlock_t *slock) __attribute__((always_inline));
