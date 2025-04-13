#pragma once

#include <lib/def.h>

typedef uint8_t spinlock_t;

#define SPINLOCK_INIT (0)

void spinlock_acquire(volatile spinlock_t *lock);

void spinlock_release(volatile spinlock_t *lock);
