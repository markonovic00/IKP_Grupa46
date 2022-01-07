#include <process.h>
#include "serverthread.h"
#include <stdio.h>

#define CAPACITY 64 //num of deliverers
#define STARTINGPORT 10000 // port start for deliverer and service communication




int main()
{
	printf("Starting deliverers...\n");
	ghMutex=CreateMutex(NULL, FALSE, NULL);

	HANDLE servers[CAPACITY];
	int ports[CAPACITY];
	for (int i = 0; i < CAPACITY; i++) {
		ports[i] = STARTINGPORT + i;
		servers[i] = (HANDLE)_beginthreadex(0, 0, &serverTherad, &ports[i], 0, 0);
	}
	WaitForMultipleObjects(CAPACITY, servers, TRUE, INFINITE);
	for (int i = 0; i < CAPACITY; i++) {
		CloseHandle(servers[i]);
	}

	return 0;
}

