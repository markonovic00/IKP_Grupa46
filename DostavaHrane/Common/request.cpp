//#include "request.h"
//#include <string.h>
//#include <stdio.h>
//#include <corecrt_malloc.h>
//
//void printList(NodeRequest* n) {
//	while (n != NULL) {
//		printf("ID: %d \n %s-%d-%d-%d\n%s %s", n->idOrder, n->foodName, n->quantity, n->price, n->urgency, n->address, n->city);
//		printf("---------------------\n");
//		n = n->next;
//	}
//}
//
//void appendList(NodeRequest** head_ref, char* _foodName, char* _address, char* _city, char _quantity, short _price, Urgency _urgency)
//{
//
//	NodeRequest* last = *head_ref;
//
//	NodeRequest* newNode = (NodeRequest*)malloc(sizeof(NodeRequest));
//
//	newNode->idOrder = 0; //Ako je prvi element u listi
//	strcpy_s(newNode->foodName, _foodName);
//	strcpy_s(newNode->address, _address);
//	strcpy_s(newNode->city, _city);
//	newNode->quantity = _quantity;
//	newNode->price = _price;
//	newNode->urgency = _urgency;
//	newNode->next = NULL;
//
//	if (*head_ref == NULL) {
//		*head_ref = newNode;
//		return;
//	}
//
//	while (last->next != NULL)
//		last = last->next;
//
//	newNode->idOrder = last->idOrder + 1;
//	last->next = newNode;
//}
////NA NEKOLIKO IZVRSENIH ZAHTEVA PROMENITI ID-OVE ISPOCEKTA
////PROBLEM INDEKSIRANJE TREDOVA
//void deleteNode(NodeRequest** head_ref, int position)
//{
//	// If linked list is empty
//	if (*head_ref == NULL)
//		return;
//
//	// Store head node
//	NodeRequest* temp = *head_ref;
//
//	// If head needs to be removed
//	if (position == 0) {
//		*head_ref = temp->next; // Change head
//		free(temp); // free old head
//		return;
//	}
//
//	// Find previous node of the node to be deleted
//	for (int i = 0; temp != NULL && i < position - 1; i++)
//		temp = temp->next;
//
//	// If position is more than number of nodes
//	if (temp == NULL || temp->next == NULL)
//		return;
//
//	// Node temp->next is the node to be deleted
//	// Store pointer to the next of node to be deleted
//	NodeRequest* next = temp->next->next;
//
//	// Unlink the node from linked list
//	free(temp->next); // Free memory
//
//	temp->next = next; // Unlink the deleted node from list
//}
//
//int countList(NodeRequest* n)
//{
//	int count = 0;
//	while (n != NULL)
//	{
//		count++;
//		n = n->next;
//	}
//
//	return count;
//}