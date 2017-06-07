#include "skynet_condition.h"

#if __X86_64__ || _M_X64

int skynet_cond_init(skynet_cond_t *cond, void *attr)
{
    cond->waiters_blocked = 0;
    cond->waiters_gone = 0;
    cond->waiters_to_unblock = 0;

    cond->block_lock = CreateSemaphoreW(NULL, 1, LONG_MAX, NULL);
    cond->block_queue = CreateSemaphoreW(NULL, 0, LONG_MAX, NULL);

    InitializeCriticalSectionAndSpinCount(&cond->unblock_lock, 4000);

    return 0;
}

int skynet_cond_wait(skynet_cond_t *cond, pthread_mutex_t *mutex)
{
    int signals_was_left = 0;
    WaitForSingleObject(cond->block_lock, INFINITE);
    ++cond->waiters_blocked;
    ReleaseSemaphore(cond->block_lock, 1, NULL);

#ifdef _MSC_VER
 #pragma inline_depth(0)
#endif

    pthread_mutex_unlock(mutex);
    WaitForSingleObject(cond->block_queue, INFINITE);

    EnterCriticalSection(&cond->unblock_lock);
    if ((signals_was_left = cond->waiters_to_unblock) != 0)
    {
        --cond->waiters_to_unblock;
    }
    else if (cond->waiters_blocked == ++cond->waiters_gone)
    {
        WaitForSingleObject(cond->block_lock, INFINITE);
        cond->waiters_blocked -= cond->waiters_gone;
        ReleaseSemaphore(cond->block_lock, 1, NULL);

        cond->waiters_gone = 0;
    }

    LeaveCriticalSection(&cond->unblock_lock);
    if (signals_was_left == 1)
    {
        ReleaseSemaphore(cond->block_lock, 1, NULL);
    }

    pthread_mutex_lock(mutex);

#ifdef _MSC_VER
 #pragma inline_depth()
#endif

    return 0;
}

static int _skynet_cond_signal(skynet_cond_t *cond, bool signal_all)
{
    int signals_to_issue = 0;
    EnterCriticalSection(&cond->unblock_lock);
    if (cond->waiters_to_unblock != 0)
    {
        if (cond->waiters_blocked == 0)
        {
            LeaveCriticalSection(&cond->unblock_lock);
            return 0;
        }
        
        if (signal_all)
        {
            cond->waiters_to_unblock += (signals_to_issue = cond->waiters_blocked);
            cond->waiters_blocked = 0;
        }
        else
        {
            signals_to_issue = 1;
            ++ cond->waiters_to_unblock;
            -- cond->waiters_blocked;
        }
    }
    else if (cond->waiters_blocked > cond->waiters_gone)
    {
        WaitForSingleObject(cond->block_lock, INFINITE);
        if (cond->waiters_gone != 0)
        {
            cond->waiters_blocked -= cond->waiters_gone;
            cond->waiters_gone = 0;
        }

        if (signal_all)
        {
            signals_to_issue = cond->waiters_to_unblock = cond->waiters_blocked;
            cond->waiters_blocked = 0;
        }
        else
        {
            signals_to_issue = cond->waiters_to_unblock = 1;
            -- cond->waiters_blocked;
        }
    }
    else
    {
        LeaveCriticalSection(&cond->unblock_lock);
        return 0;
    }

    LeaveCriticalSection(&cond->unblock_lock);
    ReleaseSemaphore(cond->block_queue, signals_to_issue, NULL);

    return 0;
}

int skynet_cond_signal(skynet_cond_t *cond)
{
    return _skynet_cond_signal(cond, false);
}

int skynet_cond_broadcast(skynet_cond_t *cond)
{
    return _skynet_cond_signal(cond, true);
}

int skynet_cond_destroy(skynet_cond_t *cond)
{
    CloseHandle(cond->block_queue);
    cond->block_queue = NULL;

    CloseHandle(cond->block_lock);
    cond->block_lock = NULL;
    DeleteCriticalSection(&cond->unblock_lock);

    return 0;
}

#endif // __x86_64__ || _M_X64
