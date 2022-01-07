#include "serverthread.h"
#include <stdio.h>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)

unsigned int __stdcall serverTherad(void* data) {

	//innerDelivererStruct* struc = (innerDelivererStruct*)data;
	//int port = struc->serverPort;
	//int clientPort = struc->clientPort;
	//struc->serverPort = htons(port);
	//struc->clientPort = htons(clientPort);
	//struc->clientSigned = FALSE;
	//char chPort[6];
	//itoa(port, chPort, 10);
	int port = *(int *)data;

	WSADATA wsa;
	SOCKET s, new_socket;
	struct sockaddr_in server, client;
	int c;
	char dataBuffer[BUFFER_SIZE];

	//printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		WaitForSingleObject(ghMutex, INFINITE);
		printf("Failed. Error Code : %d\n", WSAGetLastError());
		ReleaseMutex(ghMutex);
		return 1;
	}

	//printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		WaitForSingleObject(ghMutex, INFINITE);
		printf("Could not create socket : %d\n", WSAGetLastError());
		ReleaseMutex(ghMutex);
	}

	//printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	//Bind
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		WaitForSingleObject(ghMutex, INFINITE);
		printf("Bind failed with error code : %d\n", WSAGetLastError());
		ReleaseMutex(ghMutex);
	}

	//puts("Bind done");
	//printf("Port: %d\n", port);

	//Listen to incoming connections
	listen(s, 1);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");

	char port_c[6];
	itoa(port, port_c, 10);

	//pokretanje novog dosrtavljaca
	//ShellExecuteA(GetDesktopWindow(), "open", "C:\\FAKS\\IKP_Grupa46\\DostavaHrane\\Debug\\Deliverer.exe", port_c, NULL, SW_SHOW);

	c = sizeof(struct sockaddr_in);
	new_socket = accept(s, (struct sockaddr*)&client, &c);
	if (new_socket == INVALID_SOCKET)
	{
		WaitForSingleObject(ghMutex, INFINITE);
		printf("accept failed with error code : %d\n", WSAGetLastError());
		ReleaseMutex(ghMutex);
	}

	//neblokirajucni rezim
	//unsigned long l;
	//ioctlsocket(new_socket, FIONREAD, &l);

	//puts("Connection accepted");

	//Reply to client
	//strcpy(dataBuffer, (char*)"Hello Client , I have received your connection. But I have to go now, bye\n");
	//send(new_socket, (char*)dataBuffer, strlen(dataBuffer), 0);
	//send(new_socket, (char*)struc, (int)(sizeof(delivererStruct)), 0);

	int iResult = 1;
	delivererStruct* reply;
	printf("Waiting for connection on port %d:\n", port);
	while (true) {
		
		iResult = recv(new_socket, dataBuffer, BUFFER_SIZE, 0);

		if (iResult > 0)
		{
			//dataBuffer[iResult] = '\0';

			//primljenoj poruci u memoriji pristupiti preko pokazivaca 
			//jer znamo format u kom je poruka poslata 

			//reply = (delivererStruct*)dataBuffer;
			//if (reply->clientPort)
			//{
			//	WaitForSingleObject(ghMutex, INFINITE);
			//	//printf("HT_ITEM %s\n", htItem->key);
			//	//printf("HT_SEARCH %s\n", ht_search(ht, port_c));
			//	printf("CLient port connection: %d\n",reply->clientPort);
			//	ReleaseMutex(ghMutex);
			//	break;
			//}

		}
		else if (iResult == 0)
		{
			WaitForSingleObject(ghMutex, INFINITE);
			// connection was closed gracefully
			printf("Connection with client closed.\n");
			closesocket(new_socket);
			ReleaseMutex(ghMutex);
			break;
		}
		else
		{
			WaitForSingleObject(ghMutex, INFINITE);
			// there was an error during recv
			printf("SERVERTHREAD recv failed with error: %d\n", WSAGetLastError());
			closesocket(new_socket);
			ReleaseMutex(ghMutex);
			break;
		}
	}

	closesocket(s);
	WSACleanup();
	ReleaseMutex(ghMutex);
	return 0;
}