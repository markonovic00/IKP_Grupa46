#ifndef MYTHREAD_DOT_H
#define MYTHREAD_DOT_H
#define BUFFER_SIZE 512
#ifndef WINDOWS_DOT_H
#include <Windows.h>
#endif // !1
#ifndef REQUEST_DOT_H
#include "request.h"
#endif

#include "hashtable.h"
#include "../Common/delivery.h"


static HANDLE ghMutex;

struct threadStruct { //Listu da dodamo zahtev
	NodeRequest** head;
	NodeRequest* data;
};

struct activeStruct { //Ovo nam skida iz liste i ubacuje unutar HashTable
	NodeRequest** head;
	HashTable* ht;
	SOCKET* sender;
};

struct innerDelivererStruct { //Zadrzi podatke za komunikaciju izmedju dostavljaca i servisa i dostavljaca i klijenta...
	int serverPort;
	int clientPort;
	bool clientSigned;
	HashTable* ht;
};


unsigned int __stdcall createRequest(void* data);
unsigned int __stdcall getRequest(void* data);
unsigned int __stdcall serverTherad(void* data);
unsigned int __stdcall enlistMoreDeliverers(void* data);
unsigned int __stdcall delistDeliverers(void* data);

#endif // !THREAD_DOT_H


