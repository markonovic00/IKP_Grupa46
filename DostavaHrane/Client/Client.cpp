#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include "../Common/request.h"
#include "serverthread.h"
#include <process.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning( disable : 4996)

#pragma pack(1)

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27016
#define BUFFER_SIZE 512



// TCP client that use non-blocking sockets
int main()
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

	// Create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;								// IPv4 protocol
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
	serverAddress.sin_port = htons(SERVER_PORT);					// server port

	// Connect to server specified in serverAddress and socket connectSocket
	iResult = connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	int quantity=0;
	int hitnost=0;
	Urgency urgency;
	replyClient* reply;
	replyClient iReply;
	NodeRequest clientOrder;
	clientOrder.idOrder = htons(0);
	clientOrder.next = NULL;
	clientOrder.price = htons(0);
	clientOrder.urgency = NORMALNO;
	clientOrder.quantity = htons(0);

	int i = 0;

	HANDLE myServer;

	while (true)
	{
		printf("Unesite naziv hrane: ");
		gets_s(clientOrder.foodName, 20);

		printf("Unesite kolicinu: ");
		scanf("%d", &quantity);
		
		clientOrder.quantity = htons(quantity);

		printf("Hitnost: \n  1.Normalno\n  2.Hitno\n  Izaberite: ");
		scanf("%d", &hitnost);
		getchar();
		switch (hitnost)
		{
		case 1:
			clientOrder.urgency = NORMALNO;
			break;
		case 2:
			clientOrder.urgency = HITNO;
			break;
		default:
			clientOrder.urgency = NORMALNO;
			break;
		}

		printf("Unesite ulicu i broj: "); // proveriti unos stringa...
		scanf("%s", clientOrder.address);
		fflush(stdin);
		printf("Unesite grad: ");
		scanf("%s", clientOrder.city);
		fflush(stdin);
		
		getchar();    //pokupiti enter karakter iz bafera tastature

		printf("______________________________________________________\n");
		for (i = 0; i < 20; i++) {
			iResult = send(connectSocket, (char*)&clientOrder, (int)sizeof(NodeRequest), 0);
			Sleep(10); //Problem jednog klijenta sa puno zahteva, nekada send ne prodje sa servera, thread se zavrsi
		

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
			reply = (replyClient*)dataBuffer;
			iReply = *(replyClient*)dataBuffer;
			printf("\tPort: %d\n", ntohs(reply->port));
			printf("\tAccepted: %d\n", ntohs(reply->accepted));

			if (ntohs(reply->port) > 0) {
				printf("\tPorudzbina prihvacena, cekajte dostavljaca.\n");
				int port = ntohs(reply->port);
				myServer = (HANDLE)_beginthreadex(0, 0, &serverTherad, &port, 0, 0);
				WaitForSingleObject(myServer, INFINITE);
				CloseHandle(myServer);
			}
			printf("______________________________________________________\n");
		}
		//Sleep(50);
		}

		printf("\nPress 'x' to exit or any other key to continue: \n");
		if (getch() == 'x')
			break;
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
