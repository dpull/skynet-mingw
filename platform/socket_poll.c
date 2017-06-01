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

/*copy from skynet.*/
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
		e[i].error = false;
	}

	return n;
}

void
sp_nonblocking(int fd) {
	u_long ul = 1;
	ioctlsocket(fd, FIONBIO, &ul);
}
