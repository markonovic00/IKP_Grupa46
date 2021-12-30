#define WIN32_LEAN_AND_MEAN
#pragma warning( disable : 4996)

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "conio.h"
#include <time.h>


#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#pragma pack(1)

#define SERVER_PORT 27016
#define BUFFER_SIZE 512
#define MAX_CLIENTS 100

//Koristimo FIFO Strukturu, prvi zatrazio zahtev, prvi opsluzen.
enum Urgency {
	NORMALNO,
	HITNO,
	JAKO_HITNO,
};

struct NodeRequest {
	u_short idOrder;
	char foodName[20];
	char quantity;
	u_short price;
	Urgency urgency;
	struct NodeRequest* next;
};

void printList(NodeRequest* n);
void appendList(NodeRequest** head_ref, char* _foodName, char _quantity, u_short _price, Urgency _urgency);
void deleteNode(NodeRequest** head_ref, int position);

int main(int argc, char** argv)
{
    printf("DOGG"); //Snoop

	NodeRequest* head = NULL;
	NodeRequest* secound = NULL;
	NodeRequest* third = NULL;

	head = (NodeRequest*)malloc(sizeof(NodeRequest));
	secound = (NodeRequest*)malloc(sizeof(NodeRequest));
	third = (NodeRequest*)malloc(sizeof(NodeRequest));

	head->idOrder = 0;
	strcpy_s(head->foodName, "Kineska");
	head->quantity = 2;
	head->price = 2000;
	head->urgency = NORMALNO;
	head->next = secound;

	secound->idOrder = 1;
	strcpy_s(secound->foodName, "Mjau");
	secound->quantity = 1;
	secound->price = 1000;
	secound->urgency = HITNO;
	secound->next = third;

	third->idOrder = 2;
	strcpy_s(third->foodName, "Avvv");
	third->quantity = 3;
	third->price = 3000;
	third->urgency = JAKO_HITNO;
	third->next = NULL;

	printList(head);

	appendList(&head, (char*)"NovaHrana", 4, 2500, NORMALNO);
	printf("Lista nakon dodavanja elementa\n");
	printList(head);

	deleteNode(&head, 0);
	printf("Lista nakon skidanja elementa\n");
	printList(head);

	clock_t t;
	t = clock();

	int i = 0;
	for (i = 0; i < 20000; i++) {
		appendList(&head, (char*)"NovaHrana", 4, 2500, NORMALNO);
	}
	t = clock() - t;
	double time_taken = ((double)t) / CLOCKS_PER_SEC; // calculate the elapsed time
	printf("The program took %f seconds to execute", time_taken);

    _getch();

    return 0;
}

void printList(NodeRequest* n) {
	while (n != NULL) {
		printf("ID: %d \n %s\n%d\n%d\n%d\n", n->idOrder, n->foodName, n->quantity, n->price, n->urgency);
		printf("---------------------\n");
		n = n->next;
	}
}

void appendList(NodeRequest** head_ref, char* _foodName, char _quantity, u_short _price, Urgency _urgency)
{

	NodeRequest* last = *head_ref;

	NodeRequest* newNode = (NodeRequest*)malloc(sizeof(NodeRequest));

	newNode->idOrder = 0; //Ako je prvi element u listi
	strcpy_s(newNode->foodName, _foodName);
	newNode->quantity = _quantity;
	newNode->price = _price;
	newNode->urgency = _urgency;
	newNode->next = NULL;

	if (*head_ref == NULL) {
		*head_ref = newNode;
		return;
	}

	while (last->next != NULL)
		last = last->next;

	newNode->idOrder = last->idOrder + 1;
	last->next = newNode;
}

void deleteNode(NodeRequest** head_ref, int position)
{
	// If linked list is empty
	if (*head_ref == NULL)
		return;

	// Store head node
	NodeRequest* temp = *head_ref;

	// If head needs to be removed
	if (position == 0) {
		*head_ref = temp->next; // Change head
		free(temp); // free old head
		return;
	}

	// Find previous node of the node to be deleted
	for (int i = 0; temp != NULL && i < position - 1; i++)
		temp = temp->next;

	// If position is more than number of nodes
	if (temp == NULL || temp->next == NULL)
		return;

	// Node temp->next is the node to be deleted
	// Store pointer to the next of node to be deleted
	NodeRequest* next = temp->next->next;

	// Unlink the node from linked list
	free(temp->next); // Free memory

	temp->next = next; // Unlink the deleted node from list
}