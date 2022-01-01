#ifndef CLIENTTHREAD_DOT_H
#define CLIENTTHREAD_DOT_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning( disable : 4996)

#pragma pack(1)

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27016
#define BUFFER_SIZE 512

struct delivererCall {
	u_short port;
};

unsigned int __stdcall createClient(void* data);

#endif // !CLIENTTHREAD_DOT_H


