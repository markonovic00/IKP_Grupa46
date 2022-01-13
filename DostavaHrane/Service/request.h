#ifndef REQUEST_DOT_H
#define REQUEST_DOT_H
enum Urgency {
	NORMALNO,
	HITNO,
};

struct NodeRequest {
	short idOrder;
	char foodName[20];
	char address[20];
	char city[15];
	short quantity;
	short price;
	Urgency urgency;
	struct NodeRequest* next;
};

struct replyClient {
	short port;
	bool accepted;
};

void printList(NodeRequest* n);
void appendList(NodeRequest** head_ref, char* _foodName, char* _address, char* _city, short _quantity, short _price, Urgency _urgency);
void deleteNode(NodeRequest** head_ref, int position);
int countList(NodeRequest* n);
int findPosition(NodeRequest* n);
NodeRequest* getNode(NodeRequest* head, int position);
void deleteSameRequest(NodeRequest** n, NodeRequest* cmpNode);

#endif // !REQUEST_DOT_H


