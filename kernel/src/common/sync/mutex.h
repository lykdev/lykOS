// Implementation taken from github.com/elysium-os/cronus.
#pragma once

#include <common/sync/spinlock.h>
#include <lib/list.h>

#define MUTEX_INIT ((mutex_t) { .state = MUTEX_STATE_UNLOCKED, .lock = 0, .wait_queue = LIST_INIT })

typedef enum
{
    MUTEX_STATE_UNLOCKED,
    MUTEX_STATE_LOCKED,
    MUTEX_STATE_CONTESTED
}
mutex_state_t;

typedef struct
{
    spinlock_t lock;
    mutex_state_t state;
    list_t wait_queue;
}
mutex_t;

void mutex_acquire(mutex_t *mutex);

void mutex_release(mutex_t *mutex);
