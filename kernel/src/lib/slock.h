#pragma once

#include <lib/def.h>

typedef uint8_t slock_t;

#define SLOCK_INIT 0

void slock_acquire(volatile slock_t *lock);

void slock_release(volatile slock_t *lock);
