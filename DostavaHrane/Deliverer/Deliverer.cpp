#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include <process.h>
#include "../Common/delivery.h"
#include "clientthread.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning( disable : 4996)

#pragma pack(1)

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27016
#define BUFFER_SIZE 512


// TCP client that use non-blocking sockets
int main(int argc, char** argv)
{
	// Socket used to communicate with server
	SOCKET connectSocket = INVALID_SOCKET;

	// Variable used to store function return value
	int iResult;

	// Buffer we will use to store message
	char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	// create a socket
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	int port;
	
	
	//scanf("%d", &port);
	if (argc > 1)
		port = atoi(argv[1]);
	else
		port = 8000;
	printf("Deliverer started on port: %d\n",port);


	// Create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;								// IPv4 protocol
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
	serverAddress.sin_port = htons(port);					// server port

	// Connect to server specified in serverAddress and socket connectSocket
	iResult = connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	HANDLE clientHandle;
	delivererStruct* request;

	while (true)
	{

		iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			dataBuffer[iResult] = '\0';
			request = (delivererStruct *)dataBuffer;
			int clientPort = ntohs(request->clientPort);
			int serverPort = ntohs(request->serverPort);
			bool clientSigned = request->clientSigned;
			printf("Data: %d %d %d\n",clientPort,serverPort,clientSigned);

			//Sleep(500);
			clientHandle = (HANDLE)_beginthreadex(0, 0, &createClient, request, 0, 0);
			DWORD result=WaitForSingleObject(clientHandle, INFINITE);
			CloseHandle(clientHandle);
			printf("Waiting to receive confiramtion from client\n");
			printf("Signed:%d\n", request->clientSigned);
			if (request->clientSigned == TRUE)
			{
				iResult = send(connectSocket, (char*)request, strlen(dataBuffer), 0);
				printf("Data sent %d\n", iResult);
				break;
			}
			else
			{
				printf("Client didn't connect...\n");
				break;
			}
		}
	}

	// Shutdown the connection since we're done
	iResult = shutdown(connectSocket, SD_BOTH);

	// Check if connection is succesfully shut down.
	if (iResult == SOCKET_ERROR)
	{
		printf("Shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	Sleep(1000);

	// Close connected socket
	closesocket(connectSocket);

	// Deinitialize WSA library
	WSACleanup();

	return 0;
}
