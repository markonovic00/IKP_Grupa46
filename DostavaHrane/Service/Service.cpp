#define WIN32_LEAN_AND_MEAN
#pragma warning( disable : 4996)

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include <process.h>

#include "../Common/delivery.h"
#include "request.h"
#include "mythread.h"
#include "hashtable.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#pragma pack(1)

#define SERVER_PORT 27016
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100


// TCP server that use non-blocking sockets
int main()
{
	int maxDeliveres = 1;
	printf("Unesite maksimaln broj dostavljaca: ");
	scanf("%d", &maxDeliveres);

	int foodQuantity = 1000; //PROMENITI

	ghMutex = CreateMutex(NULL, FALSE, NULL);

	// Socket used for listening for new clients 
	SOCKET listenSocket = INVALID_SOCKET;

	// Sockets used for communication with client
	SOCKET clientSockets[MAX_CLIENTS];
	short lastIndex = 0;

	// Variable used to store function return value
	int iResult;

	// Buffer used for storing incoming data
	char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	// Initialize serverAddress structure used by bind
	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;				// IPv4 address family
	serverAddress.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
	serverAddress.sin_port = htons(SERVER_PORT);	// Use specific port

	//initialise all client_socket[] to 0 so not checked
	memset(clientSockets, 0, MAX_CLIENTS * sizeof(SOCKET));

	// Create a SOCKET for connecting to server
	listenSocket = socket(AF_INET,      // IPv4 address family
		SOCK_STREAM,  // Stream socket
		IPPROTO_TCP); // TCP protocol

	// Check if socket is successfully created
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address to socket
	iResult = bind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

	// Check if socket is successfully binded to address and port from sockaddr_in structure
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	//// All connections are by default accepted by protocol stek if socket is in listening mode.
	//// With SO_CONDITIONAL_ACCEPT parameter set to true, connections will not be accepted by default
	bool bOptVal = true;
	int bOptLen = sizeof(bool);
	iResult = setsockopt(listenSocket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char*)&bOptVal, bOptLen);
	if (iResult == SOCKET_ERROR) {
		printf("setsockopt for SO_CONDITIONAL_ACCEPT failed with error: %u\n", WSAGetLastError());
	}

	unsigned long  mode = 1;
	if (ioctlsocket(listenSocket, FIONBIO, &mode) != 0)
		printf("ioctlsocket failed with error.");

	// Set listenSocket in listening mode
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	printf("Server socket is set to listening mode. Waiting for new connection requests.\n");

	// set of socket descriptors
	fd_set readfds;

	// timeout for select function
	timeval timeVal;
	timeVal.tv_sec = 1;
	timeVal.tv_usec = 0;

	HANDLE requestsHandle[50];
	int requestsCounter = 0;
	threadStruct threadArgs;

	HANDLE createReqHandle[200];
	int createCounter = 0;
	activeStruct createArgs;
	HashTable* activeDelivery = create_table(CAPACITY);
	activeDelivery->maxDeliverers = maxDeliveres;

	NodeRequest* head = NULL;
	NodeRequest* clientOrder;

	threadArgs.head = &head;
	createArgs.ht = activeDelivery;
	createArgs.head = &head;

	while (true)
	{
		// initialize socket set
		FD_ZERO(&readfds);

		// add server's socket and clients' sockets to set
		if (lastIndex != MAX_CLIENTS)
		{
			FD_SET(listenSocket, &readfds);
		}

		for (int i = 0; i < lastIndex; i++)
		{
			FD_SET(clientSockets[i], &readfds);
		}

		// wait for events on set
		int selectResult = select(0, &readfds, NULL, NULL, &timeVal);

		if (selectResult == SOCKET_ERROR)
		{
			printf("Select failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		else if (selectResult == 0) // timeout expired
		{
			if (_kbhit()) //check if some key is pressed
			{
				_getch();
				printf("Waiting...\n");
			}
			if (activeDelivery->maxDeliverers > maxDeliveres)
			{
				//Smanjujemo dostavljace dok ne dodjemo do prvobitno zaposlenog broja
				printf("Disengane deliverer!\n");
				delistDeliverers(activeDelivery);
			}
			
			printf("Pending requests: %d!\n", countList(head));
			printf("Active requests: %d!\n", activeDelivery->count);

			if (foodQuantity <= 0)
			{
				printf("Restocking on food!\n");
				foodQuantity = 1000;
			}

			continue;
		}
		else if (FD_ISSET(listenSocket, &readfds))
		{
			// Struct for information about connected client
			sockaddr_in clientAddr;
			int clientAddrSize = sizeof(struct sockaddr_in);

			// New connection request is received. Add new socket in array on first free position.
			clientSockets[lastIndex] = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

			if (clientSockets[lastIndex] == INVALID_SOCKET)
			{
				if (WSAGetLastError() == WSAECONNRESET)
				{
					printf("accept failed, because timeout for client request has expired.\n");
				}
				else
				{
					printf("accept failed with error: %d\n", WSAGetLastError());
				}
			}
			else
			{
				if (ioctlsocket(clientSockets[lastIndex], FIONBIO, &mode) != 0)
				{
					printf("ioctlsocket failed with error.");
					continue;
				}
				lastIndex++;
				printf("New client request accepted (%d). Client address: %s : %d\n", lastIndex, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

			}
		}
		else
		{

			// Check if new message is received from connected clients
			for (int i = 0; i < lastIndex; i++)
			{
				// Check if new message is received from client on position "i"
				if (FD_ISSET(clientSockets[i], &readfds))
				{
					iResult = recv(clientSockets[i], dataBuffer, BUFFER_SIZE, 0);

					if (iResult > 0)
					{
						dataBuffer[iResult] = '\0';
						printf("Message received from client (%d):\n", i + 1);

						if (activeDelivery->count == activeDelivery->maxDeliverers) {
							printf("Engaging new deliverer!\n");
							enlistMoreDeliverers(activeDelivery);
						}

						//primljenoj poruci u memoriji pristupiti preko pokazivaca 
						//jer znamo format u kom je poruka poslata 
						clientOrder = (NodeRequest*)dataBuffer;
						clientOrder->price = 100 * ntohs(clientOrder->quantity); //ZAKUCANA CENA
						foodQuantity -= ntohs(clientOrder->quantity);

						threadArgs.data = clientOrder;
						requestsHandle[requestsCounter] = (HANDLE)_beginthreadex(0, 0, &createRequest, &threadArgs, 0, 0);
						WaitForSingleObject(requestsHandle[requestsCounter], 10);//Ako neuspesno izbaci thread
						CloseHandle(requestsHandle[requestsCounter]);
						requestsCounter++;
						if (requestsCounter >= 50)
							requestsCounter = 0;
						//printf("_______________________________BrojZahteva: %d \n", countList(head));
						
						createArgs.sender = &clientSockets[i];

						if (foodQuantity <= 0) {
							printf("No more food in stock\n");
							createArgs.ht = NULL;
						}
						else {
							createArgs.ht = activeDelivery;
						}

						createReqHandle[createCounter] = (HANDLE)_beginthreadex(0, 0, &getRequest, &createArgs, 0, 0);
						//WaitForSingleObject(createReqHandle[createCounter], INFINITE);
						CloseHandle(createReqHandle[createCounter]);
						createCounter++;
						if (createCounter >= 200)
							createCounter = 0;
						//printf("_______________________________SkinutiBrojZahteva: %d \n", countList(head));

						//printf("Message successfully sent. Total bytes: %ld\n", iResult);
						printf("______________________________________________________\n");
					}
					else if (iResult == 0)
					{
						// connection was closed gracefully
						printf("Connection with client (%d) closed.\n", i + 1);
						closesocket(clientSockets[i]);

						// sort array and clean last place
						for (int j = i; j < lastIndex - 1; j++)
						{
							clientSockets[j] = clientSockets[j + 1];
						}
						clientSockets[lastIndex - 1] = 0;

						lastIndex--;
					}
					else
					{
						// there was an error during recv
						printf("recv failed with error: %d\n", WSAGetLastError());
						closesocket(clientSockets[i]);

						// sort array and clean last place
						for (int j = i; j < lastIndex - 1; j++)
						{
							clientSockets[j] = clientSockets[j + 1];
						}
						clientSockets[lastIndex - 1] = 0;

						lastIndex--;
					}
				}
			}
		}
	}

	//Close listen and accepted sockets
	closesocket(listenSocket);

	// Deinitialize WSA library
	WSACleanup();

	return 0;
}
