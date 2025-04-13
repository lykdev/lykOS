#include "slock.h"

#include <arch/cpu.h>

#include <common/panic.h>

void spinlock_acquire(volatile spinlock_t *slock)
{
    volatile u64 deadlock_cnt = 0;

    while (true)
    {
        if (!__atomic_test_and_set(slock, __ATOMIC_ACQUIRE))
            return;

        while (__atomic_load_n(slock, __ATOMIC_RELAXED))
        {
            arch_cpu_relax();

            if (deadlock_cnt++ >= 100'000'000)
                panic("Deadlock occured. Return addr: %llx", __builtin_return_address(0));
        }
    }
}

void spinlock_release(volatile spinlock_t *slock)
{
    __atomic_clear(slock, __ATOMIC_RELEASE);
}
