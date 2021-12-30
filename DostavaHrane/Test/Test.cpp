#include <Windows.h>
#include <process.h>
#include <stdio.h>
#include <string.h>

struct NodeRequest {
	u_short idOrder;
	char foodName[20];
	char address[20];
	char city[15];
	char quantity;
	u_short price;
	struct NodeRequest* next;
};

unsigned int __stdcall mythread(void* data)
{
	NodeRequest* n = (NodeRequest*)data;
	printf("ID: %d \n %s-%d-%d\n%s %s\n", n->idOrder, n->foodName, n->quantity, n->price, n->address, n->city);
	n->price = 100;
	printf("Thread %d\n", GetCurrentThreadId());
	Sleep(500);
	return 0;
}

int main(int argc, char* argv[])
{
	NodeRequest* head = NULL;
	head = (NodeRequest*)malloc(sizeof(NodeRequest));
	head->idOrder = 0;
	strcpy_s(head->foodName, "NazivHrane");
	strcpy_s(head->address, "Ulica");
	strcpy_s(head->city, "Grad");
	head->quantity = 1;
	head->price = 200;
	head->next = NULL;

	HANDLE myhandle[100];
	DWORD threadRes[100];

	int i = 0;
	for (i = 0; i < 10; i++)
	{
		myhandle[i] = (HANDLE)_beginthreadex(0, 0, &mythread, head, 0, 0); // Pokrene thread
		//threadRes[i] = WaitForSingleObject(myhandle[i], INFINITE); // Polako ide jedan po jedan
		//if (threadRes[i] == WAIT_OBJECT_0)
			//printf("zavrsen %d\n", threadRes[i]);
	}

	for (i = 0; i < 10; i++)  // izvrsi uporedo sve
	{
		threadRes[i]=WaitForSingleObject(myhandle[i], INFINITE);
		if (threadRes[i] == WAIT_OBJECT_0)
			printf("zavrsen %d\n", threadRes[i]);
		CloseHandle(myhandle[i]);
	}

	printf("Nakon threadova\n");
	printf("ID: %d \n %s-%d-%d\n%s %s\n", head->idOrder, head->foodName, head->quantity, head->price, head->address, head->city);

	//CHECK IF THREAD STILL RUNNING
	DWORD result = WaitForSingleObject(myhandle[0], INFINITE);
	if (result == WAIT_OBJECT_0) {
		//Thread ended
	}
	else {
		//Thread still running
	}
	//END CHECK
	
	
	getchar();

	return 0;
}