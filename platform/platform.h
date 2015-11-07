#pragma once

#include <unistd.h>
#include <stdbool.h>
#include "sys/socket.h"

#define HAVE_STRUCT_TIMESPEC

#ifdef __cplusplus
extern "C" {
#endif

struct sigaction {
	void (*sa_handler)(int);
};
enum { SIGPIPE };
void sigaction(int flag, struct sigaction *action, int param);

char *strsep(char **stringp, const char *delim);

enum { CLOCK_THREAD_CPUTIME_ID, CLOCK_REALTIME, CLOCK_MONOTONIC };
int clock_gettime(int what, struct timespec *ti);

enum { LOCK_EX, LOCK_NB };

const char *inet_ntop(int af, const void *src, char *dst, size_t size); 
int kill(pid_t pid, int exit_code);
int daemon(int a, int b);
void sigaction(int flag, struct sigaction *action, int param);
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
};

bool sp_invalid(poll_fd fd);
poll_fd sp_create();
void sp_release(poll_fd fd);
int sp_add(poll_fd fd, int sock, void *ud);
void sp_del(poll_fd fd, int sock);
void sp_write(poll_fd, int sock, void *ud, bool enable);
int sp_wait(poll_fd, struct event *e, int max);
void sp_nonblocking(int sock);

int write_extend_socket(int fd, const void *ptr, size_t sz);
int read_extend_socket(int fd, void *buffer, size_t sz);
void close_extend_socket(int fd);
int pipe_socket(int fd[2]);
int connect_extend_errno(SOCKET s, const struct sockaddr* name, int namelen);

#ifndef DONOT_USE_IO_EXTEND
#define DONOT_USE_IO_EXTEND
#define write(fd, ptr, sz) write_extend_socket(fd, ptr, sz)
#define read(fd, ptr, sz)  read_extend_socket(fd, ptr, sz)
#define close(fd) close_extend_socket(fd)
#define pipe(fd) pipe_socket(fd)
#define connect(s, name, namelen) connect_extend_errno(s, name, namelen)
#endif

__declspec(dllimport) int __stdcall gethostname(char *buffer, int len);

#ifdef __cplusplus
}; /* extern "C" */
#endif
