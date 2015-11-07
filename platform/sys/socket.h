#pragma once
#define _WIN32_WINNT 0x501

#include <WinSock2.h>
#include <WS2tcpip.h>

#define EINPROGRESS WSAEWOULDBLOCK
#undef near