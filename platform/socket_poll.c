#include "platform.h"
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "epoll.h"
#include <assert.h>
#include "sys/socket.h"
#include <stdio.h>
#include <conio.h>

bool 
sp_invalid(int efd) {
	return efd == -1;
}

int
sp_create() {
	return cpoll_create(1024);
}

void
sp_release(int efd) {
	close(efd);
}

int 
sp_add(int efd, int sock, void *ud) {
	struct cpoll_event ev;
	ev.events = CPOLLIN;
	ev.data.ptr = ud;
	if (cpoll_ctl(efd, CPOLL_CTL_ADD, sock, &ev) == -1) {
		return 1;
	}
	return 0;
}

void 
sp_del(int efd, int sock) {
	cpoll_ctl(efd, CPOLL_CTL_DEL, sock , NULL);
}

void 
sp_write(int efd, int sock, void *ud, bool enable) {
	struct cpoll_event ev;
	ev.events = CPOLLIN | (enable ? CPOLLOUT : 0);
	ev.data.ptr = ud;
	cpoll_ctl(efd, CPOLL_CTL_MOD, sock, &ev);
}

int 
sp_wait(int efd, struct event *e, int max) {
	assert(max <= 1024);
	struct cpoll_event ev[1024];
	int n = cpoll_wait(efd , ev, max, -1);
	int i;
	for (i=0;i<n;i++) {
		e[i].s = ev[i].data.ptr;
		unsigned flag = ev[i].events;
		e[i].write = (flag & CPOLLOUT) != 0;
		e[i].read = (flag & CPOLLIN) != 0;
	}

	return n;
}

void
sp_nonblocking(int fd) {
	u_long ul = 1;
	ioctlsocket(fd, FIONBIO, &ul);
}

int write(int fd, const void *ptr, size_t sz) {
	if(fd == 1) {
		fwrite(ptr, sz, 1, stdout);
		return sz;
	}

	if(fd == 2) {
		fwrite(ptr, sz, 1, stderr);
		return sz;
	}	

	WSABUF vecs[1];
	vecs[0].buf = (char*)ptr;
	vecs[0].len = sz;

    DWORD bytesSent;
    if(WSASend(fd, vecs, 1, &bytesSent, 0, NULL, NULL))
        return -1;
    else
        return bytesSent;
}

int read(int fd, void *buffer, size_t sz) {
	if(fd == 0) {
		char *buf = (char *) buffer;
		while(_kbhit()) {
			char ch = _getch();
			*buf++ = ch;
			_putch(ch);
			if(ch == '\r') {
				*buf++ = '\n';
				_putch('\n');
			}
		}
		return buf - (char *) buffer;
	}

	WSABUF vecs[1];
	vecs[0].buf = (char*)buffer;
	vecs[0].len = sz;

    DWORD bytesRecv = 0;
    DWORD flags = 0;
    if(WSARecv(fd, vecs, 1, &bytesRecv, &flags, NULL, NULL)) {
		if(WSAGetLastError() == WSAECONNRESET)
			return 0;
        return -1;
	} else
        return bytesRecv;
}

void close(int fd) {
	shutdown(fd, SD_BOTH);
	closesocket(fd);
}