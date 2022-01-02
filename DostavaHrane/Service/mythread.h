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



static HANDLE ghMutex;

struct threadStruct {
	NodeRequest** head;
	NodeRequest* data;
};

struct activeStruct {
	NodeRequest** head;
	HashTable* ht;
};

unsigned int __stdcall createRequest(void* data);
unsigned int __stdcall getRequest(void* data);
unsigned int __stdcall serverTherad(void* data);

#endif // !THREAD_DOT_H


