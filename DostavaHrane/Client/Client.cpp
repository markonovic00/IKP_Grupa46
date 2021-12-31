#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma warning( disable : 4996)

#pragma pack(1)

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27016
#define BUFFER_SIZE 512


struct studentInfo {
	char ime[15];
	char prezime[20];
	short poeni;
};

struct replyClient {
	u_short port;
	bool accepted;
};

enum Urgency {
	NORMALNO,
	HITNO,
	JAKO_HITNO,
};

struct clientCall {
	char food_name[20];
	short quantity;
	Urgency urgency;
};

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

	//promenljiva tipa studentInfo cija ce se polja popunuti i cela struktira poslati u okviru jedne poruke
	studentInfo student;
	short poeni;

	clientCall order;
	short quantity;
	Urgency urgency;
	replyClient* reply;

	strcpy_s(order.food_name, "Kineska");
	order.quantity = htons(2);
	order.urgency = NORMALNO;

	while (true)
	{
		printf("Enter za slanje podatka \n");
		// Unos potrebnih podataka koji ce se poslati serveru
		//printf("Unesite ime studenta: ");
		//gets_s(student.ime, 15);

		//printf("Unesite prezime studenta: ");
		//gets_s(student.prezime, 20);

		//printf("Unesite osvojene poene na testu: ");
		//scanf("%d", &poeni);
		//student.poeni = htons(poeni);  //obavezna funkcija htons() jer cemo slati podatak tipa short 
		getchar();    //pokupiti enter karakter iz bafera tastature


		// Slanje pripremljene poruke zapisane unutar strukture studentInfo
		//prosledjujemo adresu promenljive student u memoriji, jer se na toj adresi nalaze podaci koje saljemo
		//kao i velicinu te strukture (jer je to duzina poruke u bajtima)
		iResult = send(connectSocket, (char*)&order, (int)sizeof(clientCall), 0);

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
			printf("Port: %d\n", reply->port);
			printf("Accepted: %d\n", reply->accepted);
		}

		printf("\nPress 'x' to exit or any other key to continue: ");
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
