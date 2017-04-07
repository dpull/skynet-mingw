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

#include "platform.h"
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include "sys/socket.h"
#include <stdio.h>
#include <conio.h>
#include <errno.h>

int write_extend_socket(int fd, const void* buffer, size_t sz) 
{
	int ret = send_extend_errno(fd, (const char*)buffer, sz, 0);
	if (ret == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK) 
		return write(fd, buffer, sz);
	return ret;
}

int read_extend_socket(int fd, void* buffer, size_t sz) 
{
	int ret = recv_extend_errno(fd, (char*)buffer, sz, 0);
	if (ret == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK) 
		return read(fd, buffer, sz);
	return ret;
}

int close_extend_socket(int fd) 
{
	int ret = closesocket(fd);
	if (ret == SOCKET_ERROR && WSAGetLastError() == WSAENOTSOCK) 
		return close(fd);
    return ret;
}

int pipe_socket(int fds[2]) 
{
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

int connect_extend_errno(SOCKET s, const struct sockaddr* name, int namelen)
{
	int ret = connect(s, name, namelen);
	if (ret == SOCKET_ERROR)  {
		errno = WSAGetLastError();
		if (errno == WSAEWOULDBLOCK)
			errno = EINPROGRESS;
	}
	return ret;
}

int send_extend_errno(SOCKET s, const char* buffer, int sz, int flag) 
{
	int ret = send(s, buffer, sz, flag);
	if (ret == SOCKET_ERROR)  {
		errno = WSAGetLastError();
		if (errno == WSAEWOULDBLOCK)
			errno = EAGAIN;
	}
	return ret;
}

int recv_extend_errno(SOCKET s, char* buffer, int sz, int flag) 
{
	int ret = recv(s, buffer, sz, flag);
	if (ret == SOCKET_ERROR)  {
		errno = WSAGetLastError();
		if (errno == WSAEWOULDBLOCK)
			errno = EAGAIN;
	}
	return ret;
}

int getsockopt_extend_voidptr(SOCKET s, int level, int optname, void* optval, int* optlen) 
{
    return getsockopt(s, level, optname, (char*)optval, optlen);
}

int setsockopt_extend_voidptr(SOCKET s, int level, int optname, const void* optval, int optlen)
{
    return setsockopt(s, level, optname, (char*)optval, optlen);
}


int recvfrom_extend_voidptr(SOCKET s, void* buf, int len, int flags, struct sockaddr* from, int* fromlen)
{
    int ret = recvfrom(s, (char*)buf, len, flags, from, fromlen);
	if (ret == SOCKET_ERROR)  {
		errno = WSAGetLastError();
		if (errno == WSAEWOULDBLOCK)
			errno = EAGAIN;
		if (errno == WSAECONNRESET)
			errno = EAGAIN;
	}
	return ret;
}
