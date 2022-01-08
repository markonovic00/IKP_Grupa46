#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"
#include <process.h>
#include "serverthread.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#pragma pack(1)

#define MAX_CLIENTS 200
#define SERVER_IP_ADDRESS "127.0.0.1"
#pragma warning(disable:4996)


struct studentInfo {
	char ime[15];
	char prezime[20];
	short poeni;
};

struct clientThreadArgs {
	int port;
	bool* isSigned;
};

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

unsigned int __stdcall multiServer(void* data) {

	int port = *(int*)data;
	WaitForSingleObject(ghMutex, INFINITE);
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
	serverAddress.sin_port = htons(port);	// Use specific port

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


	studentInfo* student;
	delivererStruct* request;

	clientThreadArgs args;

	HANDLE clientComms[MAX_CLIENTS];
	bool clientRes[MAX_CLIENTS];
	for (int i = 0; i < MAX_CLIENTS; i++)
		clientRes[i] = FALSE;

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
			printf("Waiting for connections... %d\n",port);
			Sleep(500);
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
					
					//printf("LastIndex %d\n", lastIndex);
					iResult = recv(clientSockets[i], dataBuffer, BUFFER_SIZE, 0);
					
					if (iResult > 0)
					{
						printf("Bytes received: %d\n", iResult);
						dataBuffer[iResult] = '\0';
						request = (delivererStruct*)dataBuffer;
						int clientPort = ntohs(request->clientPort);
						int serverPort = ntohs(request->serverPort);
						bool clientSigned = request->clientSigned;
						printf("Data: %d %d %d\n", clientPort, serverPort, clientSigned);
						args.port = clientPort;
						args.isSigned = &(clientRes[i]);
						//Mozda proveru da li je thread zavrsio uraditi spolja...
						clientComms[i] = (HANDLE)_beginthreadex(0, 0, &clientCommunication, &args, 0, 0);
						//printf("PAZITI DA LI JE ZAVRSEN NEKADA DWORD RES: %d\n", clientRes[i]);
						//clientRes[i]=WaitForSingleObject(clientComms[i], INFINITE);
						CloseHandle(clientComms[i]);
						
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
			for (int i = 0; i < lastIndex; i++) //MAXCLIENTS MOZDA.....
			{
				if (clientRes[i] == TRUE) {
					printf("--------------------------------------Dobio potvrdu\n");
					strcpy(dataBuffer, (char*)"Porudzbina je stigla. Potpis od klijenta\0");
					iResult = send(clientSockets[i], dataBuffer, (int)strlen(dataBuffer), 0);

					// Check result of send function
					if (iResult == SOCKET_ERROR)
					{
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(clientSockets[i]);
						WSACleanup();
						return 1;
					}

					printf("Message successfully sent. Total bytes: %ld\n", iResult);
					printf("Connection with client (%d) closed.\n", i + 1);
					shutdown(clientSockets[i], SD_BOTH);
					closesocket(clientSockets[i]);

					// sort array and clean last place
					for (int j = i; j < lastIndex - 1; j++)
					{
						clientSockets[j] = clientSockets[j + 1];
					}
					clientSockets[lastIndex - 1] = 0;

					lastIndex--;
					clientRes[i] = FALSE;
				}
			}
		}
	}

	//Close listen and accepted sockets
	closesocket(listenSocket);

	// Deinitialize WSA library
	WSACleanup();
	ReleaseMutex(ghMutex);

	return 0;
}

unsigned int __stdcall clientThread(void* data) {

	int port = *(int*)data;
	WaitForSingleObject(ghMutex, INFINITE);
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
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	//promenljiva tipa studentInfo cija ce se polja popunuti i cela struktira poslati u okviru jedne poruke
	studentInfo student;
	strcpy(student.ime, (char*)"ImeStud");
	strcpy(student.prezime, (char*)"PrezimeStud");
	student.poeni = 10;
	short poeni;

	innerDelivererStruct struc;//= (innerDelivererStruct*)data;
	//int port = struc->serverPort;
	//int clientPort = struc->clientPort;
	struc.serverPort = htons(1000);
	struc.clientPort = htons(10001);
	struc.clientSigned = FALSE;

	//neblokirajucni rezim
	unsigned long l;
	ioctlsocket(connectSocket, FIONREAD, &l);

	while (true)
	{
		
		//// Unos potrebnih podataka koji ce se poslati serveru
		//printf("Unesite ime studenta: ");
		//gets_s(student.ime, 15);

		//printf("Unesite prezime studenta: ");
		//gets_s(student.prezime, 20);

		//printf("Unesite osvojene poene na testu: ");
		//scanf("%d", &poeni);
		//student.poeni = htons(poeni);  //obavezna funkcija htons() jer cemo slati podatak tipa short 
		//getchar();    //pokupiti enter karakter iz bafera tastature


		// Slanje pripremljene poruke zapisane unutar strukture studentInfo
		//prosledjujemo adresu promenljive student u memoriji, jer se na toj adresi nalaze podaci koje saljemo
		//kao i velicinu te strukture (jer je to duzina poruke u bajtima)
		iResult = send(connectSocket, (char*)&struc, (int)sizeof(innerDelivererStruct), 0);

		if (iResult == -1) {
			printf("Gasi\n");
			break;
		}

		// Check result of send function
		if (iResult == SOCKET_ERROR)
		{
			printf("OVAJ PUCAsend failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		}

		printf("Message successfully sent. Total bytes: %ld\n", iResult);

		ioctlsocket(connectSocket, FIONREAD,(u_long*)&iResult);
		if (iResult > 0) {
			iResult = recv(connectSocket, dataBuffer, BUFFER_SIZE, 0);

			if (iResult > 0)
			{
				dataBuffer[iResult] = '\0';
				printf("PORUKA OD SERVERA NAKON DOBIJANJA ODGOVORA OD KLIJENTA: %s\n", dataBuffer);
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
				break;

			}
			else if (iResult == 0)
			{
				WaitForSingleObject(ghMutex, INFINITE);
				// connection was closed gracefully
				printf("Connection with client closed.\n");
				closesocket(connectSocket);
				ReleaseMutex(ghMutex);
				break;
			}
			else
			{
				WaitForSingleObject(ghMutex, INFINITE);
				// there was an error during recv
				printf("SERVERTHREAD recv failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				ReleaseMutex(ghMutex);
				break;
			}
		}

		Sleep(1000);
		//printf("\nPress 'x' to exit or any other key to continue: ");
		//if (getch() == 'x')
			//break;
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
	ReleaseMutex(ghMutex);
	return 0;
}

unsigned int __stdcall clientCommunication(void* data) {

	WaitForSingleObject(ghMutex, INFINITE);
	clientThreadArgs* struc = (clientThreadArgs*)data;
	int port = struc->port;
	bool* pok = struc->isSigned;
	printf("Port prema klijentu: %d\n", port);
	
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
		*pok = TRUE;
		printf("Unable to connect to server.\n");
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
				//(*struc).clientSigned = TRUE;
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
	ReleaseMutex(ghMutex);

	return 0;
}