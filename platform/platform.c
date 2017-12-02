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
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <errno.h>
#include <fcntl.h>
#include "epoll.h"

#define IN6ADDRSZ 16 
#define INT16SZ 2

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call,  LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH: 
    epoll_startup();
    break;

  case DLL_PROCESS_DETACH:
    epoll_cleanup();
    break;
  }
  return TRUE;
}

char *strsep(char **stringp, const char *delim) {
    char *s;
    const char *spanp;
    int c, sc;
    char *tok;
    if ((s = *stringp)== NULL)
        return (NULL);
    for (tok = s;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc =*spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}

int sigfillset(sigset_t *set) {
  /*Not implemented*/
  return 0;  
}

int sigemptyset(sigset_t *set) {
  /*Not implemented*/
  return 0;  
}

int sigaction(int sig, const struct sigaction *act, struct sigaction *oact) {
  /*Not implemented*/
  return 0;
}

int clock_gettime_platform(int what, struct timespec *ti) {
   __int64 wintime; 
   GetSystemTimeAsFileTime((FILETIME*)&wintime);
   wintime      -=116444736000000000;  //1jan1601 to 1jan1970
   ti->tv_sec  =wintime / 10000000;           //seconds
   ti->tv_nsec =wintime % 10000000 *100;      //nano-seconds
   return 0;
}

const char * inet_ntop(int af, const void *src, char *dst, size_t size) {
  if (af != AF_INET && af != AF_INET6)
    return NULL;

  SOCKADDR_STORAGE address;
  DWORD address_length;

  if (af == AF_INET)
  {
    address_length = sizeof(struct sockaddr_in);
    struct sockaddr_in* ipv4_address = (struct sockaddr_in*)(&address);
    ipv4_address->sin_family = AF_INET;
    ipv4_address->sin_port = 0;
    memcpy(&ipv4_address->sin_addr, src, sizeof(struct in_addr));
  }
  else // AF_INET6
  {
    address_length = sizeof(struct sockaddr_in6);
    struct sockaddr_in6* ipv6_address = (struct sockaddr_in6*)(&address);
    ipv6_address->sin6_family = AF_INET6;
    ipv6_address->sin6_port = 0;
    ipv6_address->sin6_flowinfo = 0;
    // hmmm
    ipv6_address->sin6_scope_id = 0;
    memcpy(&ipv6_address->sin6_addr, src, sizeof(struct in6_addr));
  }

  DWORD string_length = (DWORD)(size);
  int result;
  result = WSAAddressToStringA((struct sockaddr*)(&address), address_length, 0, dst, &string_length);
  // one common reason for this to fail is that ipv6 is not installed

  return result == SOCKET_ERROR ? NULL : dst;
}

int kill(pid_t pid, int exit_code) {
  return TerminateProcess((HANDLE)pid, exit_code);
}

int daemon(int a, int b) {
  /*Not implemented*/
  return 0;
}

int flock(int fd, int flag) {
  /*Not implemented*/
  return 0;
}

int fcntl(int fd, int cmd, long arg) {
  if (cmd == F_GETFL)
    return 0;

  if (cmd == F_SETFL && arg == O_NONBLOCK) {
    u_long ulOption = 1;
    ioctlsocket(fd, FIONBIO, &ulOption);
  }
  
  return 1;
}
