#pragma once
#define _UWIN /*disable open close*/
#include <io.h>
#undef _UWIN

#include <unistd.h>
#include <stdbool.h>

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
int pipe(int fd[2]);
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

int write(int fd, const void *ptr, size_t sz);
int read(int fd, void *buffer, size_t sz);
void close(int fd);

__declspec(dllimport) int __stdcall gethostname(char *buffer, int len);

#ifdef __cplusplus
}; /* extern "C" */
#endif
