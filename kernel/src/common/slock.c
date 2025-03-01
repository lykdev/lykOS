#include "slock.h"

#include <arch/cpu.h>
#include <common/panic.h>

void slock_acquire(volatile slock_t *lock)
{
    volatile u64 deadlock_cnt = 0;
    
    while (true)
    {
        if (!__atomic_test_and_set(lock, __ATOMIC_ACQUIRE))
            return;
        
        while(__atomic_load_n(lock, __ATOMIC_RELAXED))
        {
            arch_cpu_relax();

            if (deadlock_cnt++ >= 100'000'000)
                panic("Deadlock occured. Return addr: %llx", __builtin_return_address(0));
        }

    }
}

void slock_release(volatile slock_t *lock)
{
    __atomic_clear(lock, __ATOMIC_RELEASE);
}
