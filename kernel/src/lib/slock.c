#include "slock.h"

#include <lib/utils.h>

void slock_acquire(volatile slock *lock)
{
    volatile u64 deadlock_cnt = 0;
    while (1)
    {
        if (!__atomic_test_and_set(lock, __ATOMIC_ACQUIRE))
            return;
        
        if (deadlock_cnt++ >= 100000000)
            panic("Deadlock occured. Return addr: %llx", __builtin_return_address(0));

#if defined (__x86_64__)
        asm volatile ("pause");
#endif
    }
}

void slock_release(volatile slock *lock)
{
    __atomic_clear(lock, __ATOMIC_RELEASE);
}
