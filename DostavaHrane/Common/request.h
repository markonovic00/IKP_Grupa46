#include <WinSock2.h>
#ifndef REQUEST_DOT_H
#define REQUEST_DOT_H

enum Urgency {
	NORMALNO,
	HITNO,
};

struct NodeRequest {
	u_short idOrder;
	char foodName[20];
	char address[20];
	char city[15];
	char quantity;
	u_short price;
	Urgency urgency;
	struct NodeRequest* next;
};

void printList(NodeRequest* n);
void appendList(NodeRequest** head_ref, char* _foodName, char* _address, char* _city, char _quantity, u_short _price, Urgency _urgency);
void deleteNode(NodeRequest** head_ref, int position);
int countList(NodeRequest* n);

#endif // !REQUEST_DOT_H


