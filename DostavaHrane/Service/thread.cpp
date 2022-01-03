#include "mythread.h"
#include <stdio.h>
#include <process.h>
#pragma warning( disable : 4996)

unsigned int __stdcall createRequest(void* data) {

	threadStruct* struc = (threadStruct*)data;
	NodeRequest** head = (NodeRequest**)struc->head;
	NodeRequest* dataLoc = (NodeRequest*)struc->data;

	WaitForSingleObject(ghMutex, INFINITE);
	//for(int i=0;i<50;i++)
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
	innerDelivererStruct delivererStruc;
	HANDLE serverHandle;
	while (countList(*head) > 0 && ht->count < CAPACITY) 
	{
		

		int urgentIdx = findPosition(*head);
		getNode(*head, &retVal, urgentIdx);

		int emptyIdx = ht_get_empty_index(ht);
		int port = 10000 + emptyIdx;
		int clientPort = 10000 + emptyIdx + CAPACITY; //htsadrzi(servis_delivery_port,client_delivery_port);
		//client_delivery_port=servis_delivery_port+CAPACITY (od ht)
		char chPort[6];
		char chclientPort[6];
		itoa(port, chPort, 10);
		itoa(clientPort, chclientPort, 10);
		WaitForSingleObject(ghMutex, INFINITE);
		int insertedKey = ht_insert(ht, chPort, chclientPort);
		delivererStruc.clientPort = clientPort;
		delivererStruc.serverPort = port;
		delivererStruc.clientSigned = FALSE;
		delivererStruc.ht = ht;

		//BRISEMO SAMO AKO IMA SLOBODNIH DOSTAVLJACA
		if (insertedKey != -1)
		{
			deleteNode(head, urgentIdx); // obrisemo zahtev 
			printf("HTITEMS %d\n",ht->count);
			serverHandle = (HANDLE)_beginthreadex(0, 0, &serverTherad, &delivererStruc, 0, 0); //Svaki thread otvara svoj server, zbog iscitavanja porta da ne bude problema...
			WaitForSingleObject(serverHandle, INFINITE);
			CloseHandle(serverHandle);
		}
		//free_item(); //za brisanje podataka

		//free(retVal);

		ReleaseMutex(ghMutex);
	}
	return 0;
}


unsigned int __stdcall serverTherad(void* data) {

	innerDelivererStruct* struc = (innerDelivererStruct*)data;
	int port = struc->serverPort;
	int clientPort = struc->clientPort;
	struc->serverPort = htons(port);
	struc->clientPort = htons(clientPort);
	struc->clientSigned = FALSE;
	HashTable* ht = (HashTable*)struc->ht;
	WaitForSingleObject(ghMutex, INFINITE);
	WSADATA wsa;
	SOCKET s, new_socket;
	struct sockaddr_in server, client;
	int c;
	char dataBuffer[BUFFER_SIZE];

	//printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	//printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	//printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	//Bind
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
	}

	//puts("Bind done");
	printf("Port: %d\n", port);

	//Listen to incoming connections
	listen(s, 1);

	//Accept and incoming connection
	//puts("Waiting for incoming connections...");

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

	//neblokirajucni rezim
	//unsigned long l;
	//ioctlsocket(new_socket, FIONREAD, &l);

	//puts("Connection accepted");

	//Reply to client
	//strcpy(dataBuffer, (char*)"Hello Client , I have received your connection. But I have to go now, bye\n");
	//send(new_socket, (char*)dataBuffer, strlen(dataBuffer), 0);
	send(new_socket, (char*)struc, (int)(sizeof(delivererStruct)),0);

	int iResult=1;
	delivererStruct* reply;
	Ht_item* htItem;

	while (true) {

		iResult = recv(new_socket, dataBuffer, BUFFER_SIZE, 0);

		if (iResult > 0)
		{
			dataBuffer[iResult] = '\0';

			//primljenoj poruci u memoriji pristupiti preko pokazivaca 
			//jer znamo format u kom je poruka poslata 
			
			reply = (delivererStruct*)dataBuffer;
			if (TRUE)
			{
				htItem=ht_get_item_pointer(ht, port_c);
				//free_item(htItem); // Oslobodjeno mesto u memoriji 
				printf("ServerThreadHTITEMS: %d\n", ht->count);
				printf("HT_ITEM %s\n", htItem->key);
				printf("HT_SEARCH %s\n", ht_search(ht, port_c));
				break;
			}



			// Check result of send function
			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(new_socket);
				WSACleanup();
				return 1;
			}

			//printf("Message successfully sent. Total bytes: %ld\n", iResult);

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

	closesocket(s);
	WSACleanup();
	ReleaseMutex(ghMutex);
	return 0;
}