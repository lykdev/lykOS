#include "spinlock.h"

#include <arch/cpu.h>
#include <common/panic.h>

void spinlock_acquire(volatile spinlock_t *slock)
{
    volatile u64 deadlock_cnt = 0;

    while (true)
    {
        if (!__atomic_test_and_set(&slock->lock, __ATOMIC_ACQUIRE))
        {
            slock->prev_int_state = arch_cpu_int_enabled();
            arch_cpu_int_mask();
            return;
        }

        while (__atomic_load_n(&slock->lock, __ATOMIC_RELAXED))
        {
            arch_cpu_relax();

            if (deadlock_cnt++ >= 100'000'000)
                panic("Deadlock occured. Return addr: %llx", __builtin_return_address(0));
        }
    }
}

void spinlock_release(volatile spinlock_t *slock)
{
    bool prev_int_state = slock->prev_int_state;
    __atomic_clear(&slock->lock, __ATOMIC_RELEASE);
    if (prev_int_state)
        arch_cpu_int_unmask();
}
