#ifndef EPOLL_H
#define EPOLL_H

/* At this time, epoll on Windows only supports socket descriptors,
   not file descriptors, *and not epoll descriptors*.
   This differs from epoll's behavior of allowing any fd. */

enum EPOLL_EVENTS {
    EPOLLIN      = 0x0001,
    EPOLLOUT     = 0x0002,
    EPOLLRDHUP   = 0x0004,
    EPOLLPRI     = 0x0008,
    EPOLLERR     = 0x0010,
    EPOLLHUP     = 0x0020,
    EPOLLET      = 0x0040,
    EPOLLONESHOT = 0x0080
};

enum EPOLL_OPCODES {
    EPOLL_CTL_ADD,
    EPOLL_CTL_DEL,
    EPOLL_CTL_MOD
};

typedef union epoll_data {
    void* ptr;
    int fd;
    unsigned int u32;
    unsigned long long u64;
} epoll_data_t;

struct epoll_event {
    unsigned int events;
    epoll_data_t data;
};

/*
Function:    int epoll_startup()
Description: This function does necessary initialization for the epoll library.
             While this equates to a noop in Linux, in Windows it does not.
             Applications should call this function exactly once on startup.
             The epoll library should not be used if startup fails!
Parameters:  None
Returns:     0 if startup was successful, nonzero otherwise
*/
int epoll_startup();

/*
Function:    int epoll_create(int size)
Description: The size is a hint to the kernel regarding the size of the event backing store.
             epoll_create returns a file descriptor for use with epoll.
             This file descriptor must be closed with epoll_close.
Returns:     When successful, returns a non-negative integer identifying the descriptor.
             When an error occurs, returns -1 and errno is set appropriately.
Errors:
             CINVAL
                 size is not positive.
             CNFILE
                 The system limit on the total number of open files has been reached.
             CNOMEM
                 There was insufficient memory to create the kernel object.
*/
int epoll_create(int size);

/*
Function:    int epoll_ctl(int epfd, int opcode, int fd, struct epoll_event* event)
Description: Control a epoll descriptor, epfd, by requesting that the operation opcode
               be performed on the target file descriptor, fd. The event describes the
               object linked to the file descriptor fd.
             Returns zero on success, or -1 on error.
*/
int epoll_ctl(int epfd, int opcode, int fd, struct epoll_event* event);

/*
Function:    int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout)
Description: Wait for events on the epoll file descriptor epfd for a maximum time of timeout
               milliseconds. The memory area pointed to by events will contain the events that
               will be available for the caller. Up to maxevents are returned. The maxevents
               parameter must be greater than zero. Specifying a timeout of -1 makes epoll_wait
               wait indefinitely, while specifying a timeout equal to zero makes epoll_wait
               return immediately even if no events are available.
             Returns the number of file descriptors ready for the requested I/O, zero if
               no file descriptor became ready before the requested timeout, or -1 on error.
*/
int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);

/*
Function:    int epoll_close(int epfd)
Description: Takes a epoll file descriptor to close.
             Returns zero on success, or nonzero on error.
             Failure to close a epoll file descriptor may result in memory leaks
               and/or loss of data.
             While in Linux epoll_close() is equivalent to close(), in Windows it is not.
               To maintain portability, epoll_close must always be used for anything created
               with epoll_create.
*/
int epoll_close(int epfd);

/*
Function:    void epoll_cleanup()
Description: This function does necessary cleanup for the epoll library.
             While this equates to a noop in Linux, in Windows it does not.
             Applications should call this function exactly once before closing.
*/
void epoll_cleanup();

#endif /* EPOLL_H */

