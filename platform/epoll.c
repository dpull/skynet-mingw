/****************************************************************************
Copyright (c) 2015-2017      dpull.com
http://www.dpull.com
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "epoll.h"
#include <Winsock2.h>
#include <windows.h>
#include <conio.h>
#include <errno.h>
#include <assert.h>

struct fd_t
{
    int fd;
    struct epoll_event epoll_event;
};

struct epoll_fd
{
    int max_size;
    struct fd_set readfds;    
    struct fd_set writefds;    
    struct fd_set exceptfds;    
    struct fd_t* fds;
    CRITICAL_SECTION lock;
};

#if __x86_64__ || _M_X64
struct epfd_alloc_ctx_t
{
    int epfds_size;
    int epfds_used;
    int epfd_alloc_begin;
    CRITICAL_SECTION lock;
    struct epoll_fd **epfds;
};
static struct epfd_alloc_ctx_t *epfd_alloc_ctx = NULL;

static inline struct epfd_alloc_ctx_t *check_epfd_alloc_ctx()
{
    struct epfd_alloc_ctx_t *ctx;
    if (epfd_alloc_ctx == NULL)
    {
        ctx = (struct epfd_alloc_ctx_t *)malloc(sizeof(struct epfd_alloc_ctx_t));

        ctx->epfds_size = 10;
        ctx->epfds_used = 0;
        ctx->epfd_alloc_begin = 65536;
        ctx->epfds = (struct epoll_fd **)malloc(sizeof(struct epoll_fd *) * ctx->epfds_size);
        InitializeCriticalSectionAndSpinCount(&ctx->lock, 4000);

        epfd_alloc_ctx = ctx;
    }
    else
    {
        ctx = epfd_alloc_ctx;
    }

    if (ctx->epfds_used == ctx->epfds_size)
    {
        ctx->epfds_size += 10;
        ctx->epfds = (struct epoll_fd **)realloc(ctx->epfds, sizeof(struct epoll_fd *) * ctx->epfds_size);
    }

    return ctx;
}

static inline int alloc_epfd(struct epoll_fd *epfd_ptr)
{
    struct epfd_alloc_ctx_t *ctx = check_epfd_alloc_ctx();

    EnterCriticalSection(&ctx->lock);

    ctx->epfds[ctx->epfds_used++] = epfd_ptr;
    int epfd = ctx->epfds_used;
    
    LeaveCriticalSection(&ctx->lock);

    return epfd + ctx->epfd_alloc_begin;
}

static inline struct epoll_fd *get_epoll_fd_ptr(int epfd)
{
    struct epfd_alloc_ctx_t *ctx = check_epfd_alloc_ctx();

    epfd -= ctx->epfd_alloc_begin;
    EnterCriticalSection(&ctx->lock);
    if (epfd > ctx->epfds_used || epfd <= 0)
    {
        LeaveCriticalSection(&ctx->lock);
        return NULL;
    }

    struct epoll_fd *epfd_ptr = ctx->epfds[epfd - 1];
    LeaveCriticalSection(&ctx->lock);

    return epfd_ptr;
}
#else
#define alloc_epfd(epfd_ptr)   ((int)epfd_ptr)
#define get_epoll_fd_ptr(epfd) ((struct epoll_fd *)epfd)
#endif

int epoll_startup()
{
    WSADATA wsadata;
    return WSAStartup(MAKEWORD(2, 2), &wsadata);
}

/*
http://linux.die.net/man/2/epoll_create
*/
int epoll_create(int size)
{
    if(size < 0 || size > FD_SETSIZE) {
        errno = EINVAL;
        return 0;
    }

    struct epoll_fd* epoll_fd = (struct epoll_fd*)malloc(sizeof(*epoll_fd));
    memset(epoll_fd, 0, sizeof(*epoll_fd));

    epoll_fd->max_size = size;
    epoll_fd->fds = (struct fd_t*)malloc(sizeof(*(epoll_fd->fds)) * size);
    InitializeCriticalSectionAndSpinCount(&epoll_fd->lock, 4000);

    memset(epoll_fd->fds, 0, sizeof(*(epoll_fd->fds)) * size);
    for (int i = 0; i < size; ++i) {
        epoll_fd->fds[i].fd = INVALID_SOCKET;
    }

    return alloc_epfd(epoll_fd);
}


static int epoll_ctl_add(struct epoll_fd* epoll_fd, int fd, struct epoll_event* event)
{
    assert(!(event->events & EPOLLET));
    assert(!(event->events & EPOLLONESHOT));

    unsigned long hl;
    if (WSANtohl(fd, 1, &hl) == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK)
        return EBADF;

    for (int i = 0; i < epoll_fd->max_size; ++i) {
        assert(!(event->events & EPOLLET));
        assert(!(event->events & EPOLLONESHOT));
        
        if (epoll_fd->fds[i].fd == fd)
            return EEXIST;
    }

    for (int i = 0; i < epoll_fd->max_size; ++i) {
        struct fd_t* fd_t = epoll_fd->fds + i;
        if (fd_t->fd == INVALID_SOCKET) {
            fd_t->fd = fd;
            fd_t->epoll_event = *event;
            return 0;
        }
    }        

    return ENOMEM;    
}

static int epoll_ctl_mod(struct epoll_fd* epoll_fd, int fd, struct epoll_event* event)
{
    assert(!(event->events & EPOLLET));
    assert(!(event->events & EPOLLONESHOT));

     for (int i = 0; i < epoll_fd->max_size; ++i) {
        struct fd_t* fd_t = epoll_fd->fds + i;
        if (fd_t->fd == fd) {
            fd_t->epoll_event = *event;
            return 0;
        }
    }
    return ENOENT;   
}

static int epoll_ctl_del(struct epoll_fd* epoll_fd, int fd, struct epoll_event* event)
{
    for (int i = 0; i < epoll_fd->max_size; ++i) {
        struct fd_t* fd_t = epoll_fd->fds + i;
        if (fd_t->fd == fd) {
            fd_t->fd = INVALID_SOCKET;
            return 0;
        }
    }
    return ENOENT;   
}

/*
http://linux.die.net/man/2/epoll_ctl
*/
int epoll_ctl(int epfd, int opcode, int fd, struct epoll_event* event)
{
    int error = ENOENT;
    struct epoll_fd* epoll_fd = get_epoll_fd_ptr(epfd);
    EnterCriticalSection(&epoll_fd->lock);
    switch (opcode) {
        case EPOLL_CTL_ADD:
            error = epoll_ctl_add(epoll_fd, fd, event);
            break;
        case EPOLL_CTL_MOD:
            error = epoll_ctl_mod(epoll_fd, fd, event);
            break;  
        case EPOLL_CTL_DEL:
            error = epoll_ctl_del(epoll_fd, fd, event);
            break;           
    }
    LeaveCriticalSection(&epoll_fd->lock);

    if (error != 0)
        errno = error;
    return error != 0 ? -1 : 0;
}

static void epoll_wait_init(struct epoll_fd* epoll_fd)
{
    FD_ZERO(&epoll_fd->readfds);    
    FD_ZERO(&epoll_fd->writefds);    
    FD_ZERO(&epoll_fd->exceptfds);    

    for (int i = 0; i < epoll_fd->max_size; ++i) {
        struct fd_t* fd_t = epoll_fd->fds + i;
        if (fd_t->fd == INVALID_SOCKET)
            continue;

        if (fd_t->epoll_event.events & EPOLLIN || fd_t->epoll_event.events & EPOLLPRI)
            FD_SET(fd_t->fd, &epoll_fd->readfds);   

        if (fd_t->epoll_event.events & EPOLLOUT)
            FD_SET(fd_t->fd, &epoll_fd->writefds);  

        if (fd_t->epoll_event.events & EPOLLERR || fd_t->epoll_event.events & EPOLLRDHUP)
            FD_SET(fd_t->fd, &epoll_fd->exceptfds);   
    }    
}

static int epoll_wait_get_result(struct epoll_fd* epoll_fd, struct epoll_event* events, int maxevents)
{
    int events_index = 0;
    for (int i = 0; i < epoll_fd->max_size; ++i) {
        struct fd_t* fd_t = epoll_fd->fds + i;
        if (fd_t->fd == INVALID_SOCKET)
            continue;

        struct epoll_event* event = events + events_index;
        event->events = 0;
        if (FD_ISSET(fd_t->fd, &epoll_fd->readfds)) {
            if (fd_t->epoll_event.events & EPOLLIN)  
                event->events |= EPOLLIN;
            else if (fd_t->epoll_event.events & EPOLLPRI)  
                event->events |= EPOLLPRI;
        }

        if (FD_ISSET(fd_t->fd, &epoll_fd->writefds)) {
            event->events |= EPOLLOUT;
        }

        if (FD_ISSET(fd_t->fd, &epoll_fd->exceptfds)) {
            if (fd_t->epoll_event.events & EPOLLERR)  
                event->events |= EPOLLERR;
            else if (fd_t->epoll_event.events & EPOLLRDHUP)  
                event->events |= EPOLLRDHUP;
        }

        if (event->events != 0) {
            event->data = fd_t->epoll_event.data;
            events_index++;
            if (events_index == maxevents)
                break;
        }
    }   
    return events_index;
}

/*
http://linux.die.net/man/2/epoll_wait
*/
int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout)
{
    struct epoll_fd* epoll_fd = get_epoll_fd_ptr(epfd);

    EnterCriticalSection(&epoll_fd->lock);
    epoll_wait_init(epoll_fd);
    LeaveCriticalSection(&epoll_fd->lock);

    struct timeval wait_time = {timeout / 1000, 1000 * (timeout % 1000)};
    int total = select(1, &epoll_fd->readfds, &epoll_fd->writefds, &epoll_fd->exceptfds, timeout >= 0 ? &wait_time : NULL);
    if (total == 0)
        return 0;

    if (total < 0) {
        errno = WSAGetLastError();
        return -1;
    }

    EnterCriticalSection(&epoll_fd->lock);
    int count = epoll_wait_get_result(epoll_fd, events, maxevents);
    LeaveCriticalSection(&epoll_fd->lock);
    
    return count;
}

int epoll_close(int epfd)
{
    struct epoll_fd* epoll_fd = get_epoll_fd_ptr(epfd);
    DeleteCriticalSection(&epoll_fd->lock);
    free(epoll_fd->fds);
	free(epoll_fd);
    return 0;
}

void epoll_cleanup()
{
    WSACleanup();
}
