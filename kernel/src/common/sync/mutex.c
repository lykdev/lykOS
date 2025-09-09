#include "mutex.h"

#include <common/assert.h>
#include <lib/likely.h>
#include <lib/list.h>
#include <sys/sched.h>
#include <sys/thread.h>

#define SPIN_COUNT 10

static bool try_lock(mutex_t *mutex, bool weak)
{
    mutex_state_t state = MUTEX_STATE_UNLOCKED;
    return __atomic_compare_exchange_n(&mutex->state, &state, MUTEX_STATE_LOCKED, weak, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
}

void mutex_acquire(mutex_t *mutex)
{
    if(LIKELY(try_lock(mutex, true)))
        return;

    for(int i = 0; i < SPIN_COUNT; i++)
    {
        if(LIKELY(try_lock(mutex, true)))
            return;
        sched_yield(THREAD_STATE_READY);
    }

    spinlock_acquire(&mutex->lock);
    if(LIKELY(__atomic_exchange_n(&mutex->state, MUTEX_STATE_CONTESTED, __ATOMIC_ACQ_REL) != MUTEX_STATE_UNLOCKED))
    {
        list_append(&mutex->wait_queue, &sched_get_curr_thread()->list_node_mutex);

        spinlock_primitive_release(&mutex->lock); // Race condition here, to be fixed.
        sched_yield(THREAD_STATE_BLOCKED);
        spinlock_primitive_acquire(&mutex->lock);
    }
    else
        __atomic_store_n(&mutex->state, MUTEX_STATE_LOCKED, __ATOMIC_RELEASE);

    spinlock_release(&mutex->lock);
}

void mutex_release(mutex_t *mutex)
{
    mutex_state_t state = MUTEX_STATE_LOCKED;
    if(LIKELY(__atomic_compare_exchange_n(&mutex->state, &state, MUTEX_STATE_UNLOCKED, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)))
        return;

    spinlock_acquire(&mutex->lock);

    ASSERT(state == MUTEX_STATE_CONTESTED);
    ASSERT(!list_is_empty(&mutex->wait_queue));

    thread_t *thread = LIST_GET_CONTAINER(&mutex->wait_queue.head, thread_t, list_node_mutex);
    list_remove(&mutex->wait_queue, &thread->list_node_mutex);
    sched_enqueue(thread);

    if(list_is_empty(&mutex->wait_queue)) __atomic_store_n(&mutex->state, MUTEX_STATE_LOCKED, __ATOMIC_RELEASE);

    spinlock_release(&mutex->lock);
}
