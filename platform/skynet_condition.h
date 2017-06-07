#ifndef SKYNET_CONDITION_H
#define SKYNET_CONDITION_H

#include <pthread.h>

#if __x86_64 || _M_X64

#include <windows.h>

typedef struct 
{
    long waiters_blocked;   // Number of threads blocked.
    long waiters_gone;      // Number of threads timed out.
    long waiters_to_unblock; // Number of threads to unblock.

    HANDLE block_lock;  // Semaphore that guards access to waiters blocked count/block queue.
    HANDLE block_queue; // Queue up threads waiting for the condition to be become signalled.
    CRITICAL_SECTION unblock_lock; // Mutex that guards access to waiters (to)unblock(ed) counts.
} skynet_cond_t;

extern int skynet_cond_init(skynet_cond_t *cond, void *attr);
extern int skynet_cond_wait(skynet_cond_t *cond, pthread_mutex_t *mutex);
extern int skynet_cond_signal(skynet_cond_t *cond);
extern int skynet_cond_broadcast(skynet_cond_t *cond);
extern int skynet_cond_destroy(skynet_cond_t *cond);

#else // 32bit mode

#define skynet_cond_t pthread_cond_t

#define skynet_cond_init pthread_cond_init
#define skynet_cond_wait pthread_cond_wait
#define skynet_cond_signal pthread_cond_signal
#define skynet_cond_broadcast pthread_cond_broadcast
#define skynet_cond_destroy pthread_cond_destroy

#endif

#endif // !SKYNET_CONDITION_H
