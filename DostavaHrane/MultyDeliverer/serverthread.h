#ifndef SERVERTHREAD_DOT_H
#define SERVERTHREAD_DOT_H
#define BUFFER_SIZE 512
#ifndef WINDOWS_DOT_H
#include <Windows.h>
#endif // !1

#include "../Common/delivery.h"


static HANDLE ghMutex;

struct innerDelivererStruct {
	int serverPort;
	int clientPort;
	bool clientSigned;
};

unsigned int __stdcall serverTherad(void* data);

#endif // !SERVERTHREAD_DOT_H


