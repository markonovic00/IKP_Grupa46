#include "mythread.h"
#include <stdio.h>
#pragma warning( disable : 4996)

unsigned int __stdcall createRequest(void* data) {

	threadStruct* struc = (threadStruct*)data;
	NodeRequest** head = (NodeRequest**)struc->head;
	NodeRequest* dataLoc = (NodeRequest*)struc->data;

	WaitForSingleObject(ghMutex, INFINITE);
	//for(int i=0;i<100;i++)
		appendList(head, dataLoc->foodName, dataLoc->address, dataLoc->city, ntohs(dataLoc->quantity), dataLoc->price, dataLoc->urgency);

	printf("Thread wirting %d:\n",countList(*head));

	ReleaseMutex(ghMutex);

	return 0;
}

unsigned int __stdcall getRequest(void* data) {

	activeStruct* struc = (activeStruct*)data;
	NodeRequest** head = (NodeRequest**)struc->head;
	HashTable* ht = (HashTable*)struc->ht;
	NodeRequest* retVal = (NodeRequest*)malloc(sizeof(NodeRequest));
	while (countList(*head) > 0 && ht->count < CAPACITY) 
	{
		WaitForSingleObject(ghMutex, INFINITE);

		int urgentIdx = findPosition(*head);
		getNode(*head, &retVal, urgentIdx);

		int port = 10000 + retVal->idOrder;
		//int clientPort = 10000 + retVal->idOrder + CAPACITY; //htsadrzi(servis_delivery_port,client_delivery_port);
		//client_delivery_port=servis_delivery_port+CAPACITY (od ht)
		char chPort[6];
		itoa(port, chPort, 10);
		int insertedKey = ht_insert_auto_val(ht, chPort);// sam dodaje port u odnosu na capacity

		//BRISEMO SAMO AKO IMA SLOBODNIH DOSTAVLJACA
		if (insertedKey != -1)
			deleteNode(head, urgentIdx); // obrisemo zahtev 
		//free_item(); //za brisanje podataka

		//free(retVal);

		ReleaseMutex(ghMutex);
	}
	return 0;
}


unsigned int __stdcall serverTherad(void* data) {

	int port = *(int*)data;

	WSADATA wsa;
	SOCKET s, new_socket;
	struct sockaddr_in server, client;
	int c;
	char dataBuffer[BUFFER_SIZE];

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	//Bind
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
	}

	puts("Bind done");
	printf("Port: %d\n", port);

	//Listen to incoming connections
	listen(s, 1);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");

	char port_c[6];
	itoa(port, port_c, 10);

	//pokretanje novog dosrtavljaca
	ShellExecuteA(GetDesktopWindow(), "open", "C:\\FAKS\\IKP_Grupa46\\DostavaHrane\\Debug\\Deliverer.exe", port_c, NULL, SW_SHOW);

	c = sizeof(struct sockaddr_in);
	new_socket = accept(s, (struct sockaddr*)&client, &c);
	if (new_socket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d", WSAGetLastError());
	}


	unsigned long l;
	ioctlsocket(new_socket, FIONREAD, &l);

	puts("Connection accepted");

	//Reply to client
	strcpy(dataBuffer, (char*)"Hello Client , I have received your connection. But I have to go now, bye\n");
	send(new_socket, (char*)dataBuffer, strlen(dataBuffer), 0);

	int iResult;

	while (true) {

		iResult = recv(new_socket, dataBuffer, BUFFER_SIZE, 0);

		if (iResult > 0)
		{
			dataBuffer[iResult] = '\0';
			//printf("Message received from client (%d):\n");

			//primljenoj poruci u memoriji pristupiti preko pokazivaca 
			//jer znamo format u kom je poruka poslata 
			//order = (clientCall*)dataBuffer;

			/*printf("Naziv hrane: %s  \n", order->food_name);

			printf("Kolicina: %d  \n", ntohs(order->quantity));
			printf("Hitnost: %d \n", ntohs(order->urgency));
			printf("_______________________________ \n");
			appendList(&head, order->food_name, (char*)"Dummy", (char*)"Grad", order->quantity, 100, order->urgency);
			printf("_______________________________BrojZahteva: %d \n", countList(head));
			reply.accepted = 1;
			reply.port = 9000;*/
			iResult = send(new_socket, (char*)&dataBuffer, strlen(dataBuffer), 0);

			// Check result of send function
			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(new_socket);
				WSACleanup();
				return 1;
			}

			printf("Message successfully sent. Total bytes: %ld\n", iResult);

		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			printf("Connection with client closed.\n");
			closesocket(new_socket);
			break;
		}
		else
		{
			// there was an error during recv
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(new_socket);
			break;
		}
	}

	getchar();

	closesocket(s);
	WSACleanup();

	return 0;
}