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
	char quantity;
	short price;
	Urgency urgency;
	struct NodeRequest* next;
};

struct replyClient {
	short port;
	bool accepted;
};

#endif // !REQUEST_DOT_H


