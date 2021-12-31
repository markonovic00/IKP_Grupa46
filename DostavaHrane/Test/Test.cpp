#include <Windows.h>
#include <process.h>
#include <stdio.h>
#include <string.h>
#include "hashtable.h"

struct NodeRequest {
	u_short idOrder;
	char foodName[20];
	char address[20];
	char city[15];
	char quantity;
	u_short price;
	struct NodeRequest* next;
};

unsigned short hash(unsigned short x) {
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = (x >> 16) ^ x;
	return x;
}

struct threadStruct {
	int a;
	int b;
};

HANDLE ghMutex;

unsigned int __stdcall twoMain(void* data) {
	while (1) {
		int dt = *(int*)data;
		WaitForSingleObject(ghMutex, INFINITE); //Bez ovoga se bore za resurs, a sa time je resurs zakljucan
		printf("Trhead writing: %d\n", dt);
		Sleep(500);
		ReleaseMutex(ghMutex); // Resurs oslobodjen
	}
	return 0;
}

unsigned int __stdcall mythread(void* data)
{
	NodeRequest* n = (NodeRequest*)data;
	printf("ID: %d \n %s-%d-%d\n%s %s\n", n->idOrder, n->foodName, n->quantity, n->price, n->address, n->city);
	n->price = 100;
	printf("Thread %d\n", GetCurrentThreadId());
	Sleep(500);
	return 0;
}

unsigned int __stdcall hashThread(void* data) {
	HashTable* n = (HashTable*)data;
	char buff[10];
	_itoa_s(GetCurrentThreadId(), buff, 10); //10 decimal
	ht_insert(n, buff, (char*)"prvaPor");
	printf("Inserted \n");
	return 0;
}

int main(int argc, char* argv[])
{
	ghMutex = CreateMutex(NULL, FALSE, NULL);

	NodeRequest* head = NULL;
	head = (NodeRequest*)malloc(sizeof(NodeRequest));
	head->idOrder = 0;
	strcpy_s(head->foodName, "NazivHrane");
	strcpy_s(head->address, "Ulica");
	strcpy_s(head->city, "Grad");
	head->quantity = 1;
	head->price = 200;
	head->next = NULL;

	int i = 0;
	for (i = 9000; i < 9100; i++) {
		printf("Hash funckija za brojeve: %d\n", hash(i));
	}

	HANDLE myhandle[100];
	DWORD threadRes[100];

	for (i = 0; i < 100; i++)
	{
		myhandle[i] = (HANDLE)_beginthreadex(0, 0, &mythread, head, 0, 0); // Pokrene thread
		//threadRes[i] = WaitForSingleObject(myhandle[i], INFINITE); // Polako ide jedan po jedan
		//if (threadRes[i] == WAIT_OBJECT_0)
			//printf("zavrsen %d\n", threadRes[i]);
	}

	for (i = 0; i < 100; i++)  // izvrsi uporedo sve
	{
		threadRes[i] = WaitForSingleObject(myhandle[i], INFINITE);
		if (threadRes[i] == WAIT_OBJECT_0)
			printf("zavrsen %d\n", threadRes[i]);
		CloseHandle(myhandle[i]);
	}

	printf("Nakon threadova\n");
	printf("ID: %d \n %s-%d-%d\n%s %s\n", head->idOrder, head->foodName, head->quantity, head->price, head->address, head->city);

#pragma region HashTable

	HANDLE hashHandle[1000];

	HashTable* ht = create_table(CAPACITY);
	ht_insert(ht, (char*)"9000", (char*)"prvaPor");
	ht_insert(ht, (char*)"9001", (char*)"prvaPor");
	
	

	for (i = 0; i < 1000; i++) {
		hashHandle[i] = (HANDLE)_beginthreadex(0, 0, &hashThread, ht, 0, 0);
	}

	for (i = 0; i < 1000; i++) {
		WaitForSingleObject(hashHandle[i], 300);
		CloseHandle(hashHandle[i]);
	}
	print_table(ht);
	free_table(ht);
#pragma endregion

	HANDLE main[2];
	int a = 0;
	int b = 1;
	main[0] = (HANDLE)_beginthreadex(0, 0, &twoMain, &a, 0, 0);
	main[1]= (HANDLE)_beginthreadex(0, 0, &twoMain, &b, 0, 0);
	WaitForSingleObject(main[0], INFINITE); // RADI KOLIKO ZELIS
	WaitForSingleObject(main[1], INFINITE); // RADI KOLIKO ZELIS
	CloseHandle(main[0]);
	CloseHandle(main[1]);

	//CHECK IF THREAD STILL RUNNING
	DWORD result = WaitForSingleObject(myhandle[0], INFINITE);
	if (result == WAIT_OBJECT_0) {
		//Thread ended
	}
	else {
		//Thread still running
	}
	//END CHECK
	
	
	getchar(); // ne moze da dobije svoj red od prethodna dva threada


	return 0;
}