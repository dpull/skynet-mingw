#include "epoll.h"
#include <Winsock2.h>
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
    assert(sizeof(struct epoll_fd*) <= sizeof(int));

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

    return (int)epoll_fd;
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
    struct epoll_fd* epoll_fd = (struct epoll_fd*)epfd;
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
    struct epoll_fd* epoll_fd = (struct epoll_fd*)epfd;

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
    struct epoll_fd* epoll_fd = (struct epoll_fd*)epfd;
    DeleteCriticalSection(&epoll_fd->lock);
    free(epoll_fd->fds);
	free(epoll_fd);
    return 0;
}

void epoll_cleanup()
{
    WSACleanup();
}
