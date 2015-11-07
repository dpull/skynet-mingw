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
};

int epoll_startup()
{
    WSADATA wsadata;
    return WSAStartup(MAKEWORD(2, 2), &wsadata);
}

/*
Errors:
    EINVAL
        size is not positive.
    ENFILE
        The system limit on the total number of open files has been reached.
    ENOMEM
        There was insufficient memory to create the kernel object.
*/
int epoll_create(int size)
{
    assert(sizeof(struct epoll_fd*) <= sizeof(int));

    if(size < 0 || size > FD_SETSIZE)
        return EINVAL;

    struct epoll_fd* fd = (struct epoll_fd*)malloc(sizeof(*fd));
    memset(fd, 0, sizeof(*fd));

    fd->max_size = size;
    fd->fds = (struct fd_t*)malloc(sizeof(*(fd->fds)) * size);

    memset(fd->fds, 0, sizeof(*(fd->fds)) * size);
    for (int i = 0; i < size; ++i) {
        fd->fds[i].fd = INVALID_SOCKET;
    }

    return (int)fd;
}

/*
    EPOLL_CTL_ADD
        Add the target file descriptor fd to the epoll descriptor epfd and associate the event event with the internal file linked to fd.
    EPOLL_CTL_MOD
        Change the event event associated with the target file descriptor fd.
    EPOLL_CTL_DEL
        Remove the target file descriptor fd from the epoll file descriptor, epfd. The event is ignored and can be NULL (but see BUGS below).
Errors:
    EBADF
        epfd or fd is not a valid file descriptor.
    EEXIST
        op was EPOLL_CTL_ADD, and the supplied file descriptor fd is already in epfd.
    EINVAL
        epfd is not an epoll file descriptor, or fd is the same as epfd, or the requested operation op is not supported by this interface.
    ENOENT
        op was EPOLL_CTL_MOD or EPOLL_CTL_DEL, and fd is not in epfd.
    ENOMEM
        There was insufficient memory to handle the requested op control operation.
    EPERM
        The target file fd does not support epoll.
*/
int epoll_ctl(int epfd, int opcode, int fd, struct epoll_event* event)
{
    struct epoll_fd* efd = (struct epoll_fd*)epfd;
    if(opcode == EPOLL_CTL_ADD) {
        assert(!(event->events & EPOLLET));
        assert(!(event->events & EPOLLONESHOT));

        long hl;
        if (WSANtohl(fd, 1, &hl) == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK)
            return EBADF;

        for (int i = 0; i < efd->max_size; ++i) {
            assert(!(event->events & EPOLLET));
            assert(!(event->events & EPOLLONESHOT));
            
            if (efd->fds[i].fd == fd)
                return EEXIST;
        }

        for (int i = 0; i < efd->max_size; ++i) {
            struct fd_t* fd_t = efd->fds + i;
            if (fd_t->fd == INVALID_SOCKET) {
                fd_t->fd = fd;
                fd_t->epoll_event = *event;
                return 0;
            }
        }        

        return ENOMEM;
    }
    else if(opcode == EPOLL_CTL_MOD) {
        for (int i = 0; i < efd->max_size; ++i) {
            struct fd_t* fd_t = efd->fds + i;
            if (fd_t->fd == fd) {
                fd_t->epoll_event = *event;
                return 0;
            }
        }
        return ENOENT;
    }
    else if(opcode == EPOLL_CTL_DEL) {
        for (int i = 0; i < efd->max_size; ++i) {
            struct fd_t* fd_t = efd->fds + i;
            if (fd_t->fd == fd) {
                fd_t->fd = INVALID_SOCKET;
                return 0;
            }
        }
        return ENOENT;
    }

    return EINVAL;
}

int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout)
{
    struct epoll_fd* fd = (struct epoll_fd*)epfd;

    FD_ZERO(&fd->readfds);    
    FD_ZERO(&fd->writefds);    
    FD_ZERO(&fd->exceptfds);    

    for (int i = 0; i < fd->max_size; ++i) {
        struct fd_t* fd_t = fd->fds + i;
        if (fd_t->fd == INVALID_SOCKET)
            continue;

        if (fd_t->epoll_event.events & EPOLLIN || fd_t->epoll_event.events & EPOLLPRI)
            FD_SET(fd_t->fd, &fd->readfds);   

        if (fd_t->epoll_event.events & EPOLLOUT)
            FD_SET(fd_t->fd, &fd->writefds);  

        if (fd_t->epoll_event.events & EPOLLERR || fd_t->epoll_event.events & EPOLLRDHUP)
            FD_SET(fd_t->fd, &fd->exceptfds);   
    }    

    struct timeval wait_time = {timeout / 1000, 1000 * (timeout % 1000)};
    int total = select(1, &fd->readfds, &fd->writefds, &fd->exceptfds, timeout >= 0 ? &wait_time : NULL);  
    if (total == 0)
        return 0;

    if (total < 0) {
        errno = WSAGetLastError();
        printf("epoll_wait %d\n",  WSAGetLastError());
        return -1;
    }

    int events_index = 0;
    for (int i = 0; i < fd->max_size; ++i) {
        struct fd_t* fd_t = fd->fds + i;
        if (fd_t->fd == INVALID_SOCKET)
            continue;

        struct epoll_event* event = events + events_index;
        event->events = 0;
        if (FD_ISSET(fd_t->fd, &fd->readfds)) {
            if (fd_t->epoll_event.events & EPOLLIN)  
                event->events |= EPOLLIN;
            else if (fd_t->epoll_event.events & EPOLLPRI)  
                event->events |= EPOLLPRI;
        }

        if (FD_ISSET(fd_t->fd, &fd->writefds)) {
            event->events |= EPOLLOUT;
        }

        if (FD_ISSET(fd_t->fd, &fd->exceptfds)) {
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

int epoll_close(int epfd)
{
    struct epoll_fd* fd = (struct epoll_fd*)epfd;
    free(fd->fds);
	free(fd);
    return 0;
}

void epoll_cleanup()
{
    WSACleanup();
}
