#include "reaper.h"

#include <mm/heap.h>
#include <sys/proc.h>
#include <sys/thread.h>
#include <tasking/sched.h>

static list_t threads_queue = LIST_INIT;
static spinlock_t slock_threads = SPINLOCK_INIT;

void reaper_enqueue_thread(thread_t *t)
{
    spinlock_acquire(&slock_threads);

    list_append(&threads_queue, &t->list_node_sched);

    spinlock_release(&slock_threads);
}

static void reaper_main()
{
    while (true)
    {
        spinlock_acquire(&slock_threads);

        FOREACH(n, threads_queue)
        {
            thread_t *t = LIST_GET_CONTAINER(n, thread_t, list_node_proc);
            if (t->ref_count != 0)
                continue;

            spinlock_acquire(&t->parent_proc->slock);
            if (t->ref_count != 0)
            {
                spinlock_release(&t->parent_proc->slock);
                continue;
            }
            list_remove(&t->parent_proc->threads, &t->list_node_proc);
            spinlock_release(&t->parent_proc->slock);

            heap_free(t);
        }

        spinlock_release(&slock_threads);
    }
}

void reaper_init()
{
    proc_t *proc = proc_new(PROC_KERNEL);
    thread_new(proc, (uptr)&reaper_main);

    sched_enqueue(LIST_GET_CONTAINER(proc->threads.head, thread_t, list_node_proc));
}
