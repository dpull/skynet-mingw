#include "platform.h"
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "epoll.h"
#include <assert.h>
#include "sys/socket.h"
#include <stdio.h>
#include <conio.h>
#include <errno.h>

bool 
sp_invalid(int efd) {
	return efd == -1;
}

int
sp_create() {
	return epoll_create(1024);
}

void
sp_release(int efd) {
	closesocket(efd);
}

int 
sp_add(int efd, int sock, void *ud) {
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.ptr = ud;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, sock, &ev) == -1) {
		return 1;
	}
	return 0;
}

void 
sp_del(int efd, int sock) {
	epoll_ctl(efd, EPOLL_CTL_DEL, sock , NULL);
}

void 
sp_write(int efd, int sock, void *ud, bool enable) {
	struct epoll_event ev;
	ev.events = EPOLLIN | (enable ? EPOLLOUT : 0);
	ev.data.ptr = ud;
	epoll_ctl(efd, EPOLL_CTL_MOD, sock, &ev);
}

int 
sp_wait(int efd, struct event *e, int max) {
	assert(max <= 1024);
	struct epoll_event ev[1024];
	int n = epoll_wait(efd , ev, max, -1);
	int i;
	for (i=0; i<n;i++) {
		e[i].s = ev[i].data.ptr;
		unsigned flag = ev[i].events;
		e[i].write = (flag & EPOLLOUT) != 0;
		e[i].read = (flag & EPOLLIN) != 0;
	}

	return n;
}

void
sp_nonblocking(int fd) {
	u_long ul = 1;
	ioctlsocket(fd, FIONBIO, &ul);
}

int 
write_extend_socket(int fd, const void *buffer, size_t sz) {
	int ret = send_extend_errno(fd, (const char*)buffer, sz, 0);
	if (ret == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK) 
		return write(fd, buffer, sz);
	return ret;
}

int 
read_extend_socket(int fd, void *buffer, size_t sz) {
	int ret = recv_extend_errno(fd, (char*)buffer, sz, 0);
	if (ret == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK) 
		return read(fd, buffer, sz);

	if (ret == 0) {
		printf("%d\n", WSAGetLastError());
	}
	return ret;
}

void 
close_extend_socket(int fd) {
	int ret = closesocket(fd);
	if (ret == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK) 
		close(fd);
}

int 
pipe_socket(int fds[2]) {
    struct sockaddr_in name;
    int namelen = sizeof(name);
    SOCKET server = INVALID_SOCKET;
    SOCKET client1 = INVALID_SOCKET;
    SOCKET client2 = INVALID_SOCKET;

    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET)
        goto failed;

    if (bind(server, (struct sockaddr*)&name, namelen) == SOCKET_ERROR) 
        goto failed;

    if (listen(server, 5) == SOCKET_ERROR)
        goto failed;

    if(getsockname(server, (struct sockaddr*)&name, &namelen) == SOCKET_ERROR)
        goto failed;

    client1 = socket(AF_INET, SOCK_STREAM, 0);
    if (client1 == INVALID_SOCKET)
        goto failed;

    if (connect(client1, (struct sockaddr*)&name, namelen) == SOCKET_ERROR)
        goto failed;

    client2 = accept(server, (struct sockaddr*)&name, &namelen);
    if (client2 == INVALID_SOCKET)
        goto failed;

    closesocket(server);
    fds[0] = client1;
    fds[1] = client2;
    return 0;

failed:
    if (server != INVALID_SOCKET)
        closesocket(server);

    if (client1 != INVALID_SOCKET)
        closesocket(client1);

    if (client2 != INVALID_SOCKET)
        closesocket(client2);
    return -1;
}

int 
connect_extend_errno(SOCKET s, const struct sockaddr* name, int namelen) {
	int ret = connect(s, name, namelen);
	if (ret == SOCKET_ERROR)  {
		errno = WSAGetLastError();
		if (errno == WSAEWOULDBLOCK)
			errno = EINPROGRESS;
	}
	return ret;
}

int 
send_extend_errno(SOCKET s, const char* buffer, int sz, int flag) {
	int ret = send(s, buffer, sz, flag);
	if (ret == SOCKET_ERROR)  {
		errno = WSAGetLastError();
		if (errno == WSAEWOULDBLOCK)
			errno = EAGAIN;
	}
	return ret;
}

int 
recv_extend_errno(SOCKET s, char* buffer, int sz, int flag) {
	int ret = recv(s, buffer, sz, flag);
	if (ret == SOCKET_ERROR)  {
		errno = WSAGetLastError();
		if (errno == WSAEWOULDBLOCK)
			errno = EAGAIN;
	}
	return ret;
}
