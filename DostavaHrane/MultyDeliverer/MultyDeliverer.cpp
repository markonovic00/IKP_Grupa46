#include <process.h>
#include <Windows.h>
#include "serverthread.h"
#include <stdio.h>

#define LISTENINGPORT 30000 // port start for deliverer multiServers




int main()
{
	printf("Starting deliverers...\n");
	ghMutex=CreateMutex(NULL, FALSE, NULL);

	HANDLE server;
	int port = LISTENINGPORT;
	server = (HANDLE)_beginthreadex(0, 0, &multiServer, &port, 0, 0);
	//WaitForSingleObject(server, INFINITE);
	CloseHandle(server);

	int clientsActive = 1;
	HANDLE clients[1];
	for (int i = 0; i < clientsActive; i++) {
		clients[i] = (HANDLE)_beginthreadex(0, 0, &clientThread, &port, 0, 0);
	}
	WaitForMultipleObjects(1, clients, TRUE, INFINITE);

	for (int i = 0; i < clientsActive; i++) {
		CloseHandle(clients[i]);
	}

	getchar();

	return 0;
}

