#ifndef PLATFORM_H
#define PLATFORM_H

#include "sys/socket.h"
#include <stdbool.h>

#define random rand
#define srandom srand

#ifndef socket_poll_h
#define socket_poll_h
#endif

typedef int poll_fd;

/*skynet/skynet-src/socket_poll.h*/
struct event {
	void * s;
	bool read;
	bool write;
	bool error;
	bool eof;
};

bool sp_invalid(poll_fd fd);
poll_fd sp_create();
void sp_release(poll_fd fd);
int sp_add(poll_fd fd, int sock, void *ud);
void sp_del(poll_fd fd, int sock);
void sp_write(poll_fd, int sock, void *ud, bool enable);
int sp_wait(poll_fd, struct event *e, int max);
void sp_nonblocking(int sock);

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
