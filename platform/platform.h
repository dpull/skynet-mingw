#ifndef PLATFORM_H
#define PLATFORM_H

#include "sys/socket.h"
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define HAVE_STRUCT_TIMESPEC

/*<signal.h>*/
#define	SIGHUP	1
#define SA_RESTART	0x0002
struct sigaction {
	void (*sa_handler)(int);
	sigset_t sa_mask;              
    int sa_flags;    
};
enum { SIGPIPE };

int sigfillset(sigset_t *set);
int sigemptyset(sigset_t *set);
int sigaction(int sig, const struct sigaction *act, struct sigaction *oact);
/*<signal.h>*/

char *strsep(char **stringp, const char *delim);

enum { CLOCK_THREAD_CPUTIME_ID, CLOCK_REALTIME, CLOCK_MONOTONIC };
#define clock_gettime clock_gettime_platform
int clock_gettime_platform(int what, struct timespec *ti);

enum { LOCK_EX, LOCK_NB };

const char *inet_ntop(int af, const void *src, char *dst, size_t size); 
int kill(pid_t pid, int exit_code);
int daemon(int a, int b);
int flock(int fd, int flag);

#define O_NONBLOCK 1
#define F_SETFL 0
#define F_GETFL 1

int fcntl(int fd, int cmd, long arg); 

#define random rand
#define srandom srand

#ifndef socket_poll_h
#define socket_poll_h
#endif

typedef int poll_fd;

struct event {
	void * s;
	bool read;
	bool write;
	bool error;
};

bool sp_invalid(poll_fd fd);
poll_fd sp_create();
void sp_release(poll_fd fd);
int sp_add(poll_fd fd, int sock, void *ud);
void sp_del(poll_fd fd, int sock);
void sp_write(poll_fd, int sock, void *ud, bool enable);
int sp_wait(poll_fd, struct event *e, int max);
void sp_nonblocking(int sock);

int write_extend_socket(int fd, const void *buffer, size_t sz);
int read_extend_socket(int fd, void *buffer, size_t sz);
int close_extend_socket(int fd);
int pipe_socket(int fd[2]);
int connect_extend_errno(SOCKET s, const struct sockaddr* name, int namelen);
int send_extend_errno(SOCKET s, const char* buffer, int sz, int flag);
int recv_extend_errno(SOCKET s, char* buffer, int sz, int flag);
int recv_extend_errno(SOCKET s, char* buffer, int sz, int flag);
int getsockopt_extend_voidptr(SOCKET s, int level, int optname, void* optval, int* optlen);
int setsockopt_extend_voidptr(SOCKET s, int level, int optname, const void* optval, int optlen);
int recvfrom_extend_voidptr(SOCKET s, void* buf, int len, int flags, struct sockaddr* from, int* fromlen);

#ifndef DONOT_USE_IO_EXTEND
#define write(fd, ptr, sz) write_extend_socket(fd, ptr, sz)
#define read(fd, ptr, sz)  read_extend_socket(fd, ptr, sz)
#define close(fd) close_extend_socket(fd)
#define pipe(fd) pipe_socket(fd)
#define connect(s, name, namelen) connect_extend_errno(s, name, namelen)
#define send(s, buffer, sz, flag) send_extend_errno(s, buffer, sz, flag)
#define recv(s, buffer, sz, flag) recv_extend_errno(s, buffer, sz, flag)
#define getsockopt(s, level, optname, optval, optlen) getsockopt_extend_voidptr(s, level, optname, optval, optlen)
#define setsockopt(s, level, optname, optval, optlen) setsockopt_extend_voidptr(s, level, optname, optval, optlen)
#define recvfrom(s, buf, len, flags, from, fromlen) recvfrom_extend_voidptr(s, buf, len, flags, from, fromlen)
#endif

__declspec(dllimport) int __stdcall gethostname(char *buffer, int len);

#endif
