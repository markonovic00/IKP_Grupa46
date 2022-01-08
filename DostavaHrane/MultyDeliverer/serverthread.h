#ifndef SERVERTHREAD_DOT_H
#define SERVERTHREAD_DOT_H
#define BUFFER_SIZE 512
#ifndef WINDOWS_DOT_H

#endif // !1

#include "../Common/delivery.h"


static HANDLE ghMutex;

struct innerDelivererStruct {
	int serverPort;
	int clientPort;
	bool clientSigned;
};

unsigned int __stdcall serverTherad(void* data);
unsigned int __stdcall multiServer(void* data);
unsigned int __stdcall clientThread(void* data);
unsigned int __stdcall clientCommunication(void* data);

#endif // !SERVERTHREAD_DOT_H


