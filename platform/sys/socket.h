#pragma once
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x501
#endif 

#include <WinSock2.h>
#include <WS2tcpip.h>

#define EINPROGRESS WSAEWOULDBLOCK
#undef near