#include "clientthread.h"
#include <WinSock2.h>
#include <stdio.h>
#include <conio.h>
#pragma warning( disable : 4996)

#define BUFFER_SIZE 512
#define SERVER_IP_ADDRESS "127.0.0.1"

unsigned int __stdcall createClient(void* data) {

	delivererStruct* struc = (delivererStruct*)data;
	int port = ntohs(struc->clientPort);
	printf("Port prema klijentu: %d\n",port);
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

	// Create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;								// IPv4 protocol
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
	serverAddress.sin_port = htons(port);					// server port

	// Connect to server specified in serverAddress and socket connectSocket
	iResult = connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		printf("Unable to connect to server.Trhead writing\n");
		(*struc).clientSigned = FALSE;
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	strcpy(dataBuffer, (char*)"Porudzbina je stigla. Potpis od klijenta\0");

	while (true)
	{

		iResult = send(connectSocket, dataBuffer, strlen(dataBuffer), 0);

		// Check result of send function
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		}

		printf("Message successfully sent. Total bytes: %ld\n", iResult);

		iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			dataBuffer[iResult] = '\0';
			printf("Primljena poruka od klijenta: %s\n", dataBuffer);
			if (strcmp(dataBuffer, "Da\n") == 0)
			{
				(*struc).clientSigned = TRUE;
				break;
			}
			//printf("Doslo je do greske pri predaji paketa\n");
			//break;
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