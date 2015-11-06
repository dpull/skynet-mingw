#include "platform.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <errno.h>
#include <fcntl.h>

#define IN6ADDRSZ 16 
#define INT16SZ 2

extern "C" BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call,  LPVOID lpReserved) {
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);
    break;

  case DLL_PROCESS_DETACH:
    WSACleanup();
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

int clock_gettime(int what, struct timespec *ti) {
   __int64 wintime; 
   GetSystemTimeAsFileTime((FILETIME*)&wintime);
   wintime      -=116444736000000000;  //1jan1601 to 1jan1970
   ti->tv_sec  =wintime / 10000000;           //seconds
   ti->tv_nsec =wintime % 10000000 *100;      //nano-seconds
   return 0;
}

static const char * inet_ntop_v4(const void *src, char *dst, size_t size) {
  const char digits[] = "0123456789";
  int i;
  struct in_addr *addr = (struct in_addr *)src;
  u_long a = ntohl(addr->s_addr);
  const char *orig_dst = dst;

  if (size < INET_ADDRSTRLEN) {
    errno = ENOSPC;
    return NULL;
  }

  for (i = 0; i < 4; ++i) {
    int n = (a >> (24 - i * 8)) & 0xFF;
    int non_zerop = 0;
    
    if (non_zerop || n / 100 > 0) {
      *dst++ = digits[n / 100];
      n %= 100;
      non_zerop = 1;
    }

    if (non_zerop || n / 10 > 0) {
      *dst++ = digits[n / 10];
      n %= 10;
      non_zerop = 1;
    }
    *dst++ = digits[n];
    if (i != 3)
      *dst++ = '.';
  }

  *dst++ = '\0';
  return orig_dst;
}

const char * inet_ntop(int af, const void *src, char *dst, size_t size) {
  switch (af) {
  case AF_INET:
    return inet_ntop_v4(src, dst, size);

  default:
    return NULL;
  }
}

int pipe(int fd[2]) {
  return  _pipe(fd,4096, _O_BINARY);
}

int kill(pid_t pid, int exit_code) {
  return TerminateProcess((HANDLE)pid, exit_code);
}

int daemon(int a, int b) {
  /*Not implemented*/
  return 0;
}

void sigaction(int flag, struct sigaction *action, int param) {
  /*Not implemented*/
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
}
