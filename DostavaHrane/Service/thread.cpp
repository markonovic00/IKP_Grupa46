#include "mythread.h"
#include <stdio.h>
#include <process.h>
#include <time.h>
#pragma warning( disable : 4996)

unsigned int __stdcall createRequest(void* data) {

	threadStruct* struc = (threadStruct*)data;
	NodeRequest** head = (NodeRequest**)struc->head;
	NodeRequest* dataLoc = (NodeRequest*)struc->data;

	WaitForSingleObject(ghMutex, INFINITE);
	//for(int i=0;i<5;i++)
		appendList(head, dataLoc->foodName, dataLoc->address, dataLoc->city, ntohs(dataLoc->quantity), dataLoc->price, dataLoc->urgency);

	//printf("createRequestThread writing listCount: %d\n",countList(*head));

	ReleaseMutex(ghMutex);

	return 0;
}

unsigned int __stdcall getRequest(void* data) {

	clock_t begin = clock();
	WaitForSingleObject(ghMutex, INFINITE);
	activeStruct* struc = (activeStruct*)data;
	NodeRequest** head = (NodeRequest**)struc->head;
	HashTable* ht = (HashTable*)struc->ht;
	SOCKET* clientSocket = (SOCKET*)struc->sender;
	NodeRequest* retVal = (NodeRequest*)malloc(sizeof(NodeRequest));
	replyClient reply;
	int iResult = 0;
	innerDelivererStruct delivererStruc;
	HANDLE serverHandle;
	while (countList(*head) > 0 && ht->count < ht->maxDeliverers)
	{
		//WaitForSingleObject(ghMutex, INFINITE);
		printf("GetRequestThread Writing mutex \n");
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
		int insertedKey = ht_insert(ht, chPort, chclientPort);
		delivererStruc.clientPort = clientPort;
		delivererStruc.serverPort = port;
		delivererStruc.clientSigned = FALSE;
		delivererStruc.ht = ht;
		reply.port = htons(clientPort);
		reply.accepted = htons(1);

		//PROBLEM MOGUCI JE TO STO PRI VECEM BROJU ZAHTEVA KLINET NEKADA NE DOBIJE PORT JER SE THREAD PREBRZO ZVRSI
		printf("Poslato klijentu port\n");
		iResult = send(*clientSocket, (char*)&reply, (int)sizeof(replyClient), 0);

		// Check result of send function
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(*clientSocket);
			WSACleanup();
			return 1;
		}
		
		//BRISEMO SAMO AKO IMA SLOBODNIH DOSTAVLJACA
		if (insertedKey != -1)
		{
			deleteNode(head, urgentIdx); // obrisemo zahtev 
			deleteSameRequest(head, retVal);//Obrisemo isti zahtev ako je ista adresa, jer dostavljac nosi na istu adresu
			ReleaseMutex(ghMutex);
			serverHandle = (HANDLE)_beginthreadex(0, 0, &serverTherad, &delivererStruc, 0, 0); //Svaki thread otvara svoj server, zbog iscitavanja porta da ne bude problema...
			WaitForSingleObject(serverHandle, INFINITE);
			CloseHandle(serverHandle);
		}

		clock_t end = clock();
		double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		printf("Time INSIDE SERVER CREATOR THREAD: %f s\n",time_spent);

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
	char chPort[6];
	itoa(port, chPort, 10);
	
	WSADATA wsa;
	SOCKET s, new_socket;
	struct sockaddr_in server, client;
	int c;
	char dataBuffer[BUFFER_SIZE];

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		WaitForSingleObject(ghMutex, INFINITE);
		printf("Failed. Error Code : %d", WSAGetLastError());
		ReleaseMutex(ghMutex);
		return 1;
	}

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		WaitForSingleObject(ghMutex, INFINITE);
		printf("Could not create socket : %d", WSAGetLastError());
		ReleaseMutex(ghMutex);
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	//Bind
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		WaitForSingleObject(ghMutex, INFINITE);
		printf("Bind failed with error code : %d", WSAGetLastError());
		ReleaseMutex(ghMutex);
	}

	//puts("Bind done");
	//printf("Port: %d\n", port);

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
		WaitForSingleObject(ghMutex, INFINITE);
		printf("accept failed with error code : %d", WSAGetLastError());
		ReleaseMutex(ghMutex);
	}

	//neblokirajucni rezim
	//unsigned long l;
	//ioctlsocket(new_socket, FIONREAD, &l);

	//puts("Connection accepted");
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
			if (reply->clientSigned)
			{
				WaitForSingleObject(ghMutex, INFINITE);
				ht_set_item_NULL(ht, chPort);
				(*ht).count--;
				//printf("ServerThreadHTITEMS: %d\n", ht->count);
				ReleaseMutex(ghMutex);
				break;
			}

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


unsigned int __stdcall enlistMoreDeliverers(void* data) {

	WaitForSingleObject(ghMutex, 10);
	HashTable* ht = (HashTable*)data;
	if(ht->maxDeliverers+3<=CAPACITY)
	(*ht).maxDeliverers += 3;
	ReleaseMutex(ghMutex);

	return 0;
}

unsigned int __stdcall delistDeliverers(void* data) {
	WaitForSingleObject(ghMutex, 10);
	HashTable* ht = (HashTable*)data;

	(*ht).maxDeliverers -= 1;
	ReleaseMutex(ghMutex);

	return 0;
}