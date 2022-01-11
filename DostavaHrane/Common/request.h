#ifndef REQUEST_DOT_H
#define REQUEST_DOT_H
enum Urgency { //Koristi klijent
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

struct replyClient { //Koristi klijent da izvuce odgovor o potvrdi od strane servera
	short port;
	bool accepted;
};

#endif // !REQUEST_DOT_H


