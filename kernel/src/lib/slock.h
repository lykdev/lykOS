#pragma once

typedef char slock;

#define SLOCK_INIT 0

void slock_acquire(volatile slock *lock);

void slock_release(volatile slock *lock);

