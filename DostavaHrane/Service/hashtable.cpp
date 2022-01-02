#include "hashtable.h"
#include <stdlib.h>
#include <stdio.h>
#pragma warning( disable : 4996)

unsigned long hash_function(unsigned i) {
	return i % CAPACITY;
}
unsigned long hash_function(char* str) {
	unsigned long i = 0;
	for (int j = 0; str[j]; j++)
		i += str[j];
	return i % CAPACITY;
}

Ht_item* create_item(char* key, char* value) {
	Ht_item* item = (Ht_item*)malloc(sizeof(Ht_item));
	item->key = (char*)malloc(strlen(key) + 1);
	item->value = (char*)malloc(strlen(value) + 1);

	strcpy(item->key, key);
	strcpy(item->value, value);

	return item;
}

HashTable* create_table(int size) {
	HashTable* table = (HashTable*)malloc(sizeof(HashTable));
	table->size = size;
	table->count = 0;
	table->items = (Ht_item**)calloc(table->size, sizeof(Ht_item*));
	for (int i = 0; i < table->size; i++)
		table->items[i] = NULL;

	return table;
}

void free_item(Ht_item* item) {
	free(item->key);
	free(item->value);
	free(item);
}
void free_table(HashTable* table) {
	for (int i = 0; i < table->size; i++) {
		Ht_item* item = table->items[i];
		if (item != NULL)
			free_item(item);
	}

	free(table->items);
	free(table);
}

char* ht_search(HashTable* table, char* key) {
	int index = hash_function(atoi(key));
	Ht_item* item = table->items[index];

	if (item != NULL) {
		if (strcmp(item->key, key) == 0)
			return item->value;
	}
	return NULL;
}

Ht_item* ht_get_item_pointer(HashTable* table, char* key) {
	int index = hash_function(atoi(key));
	return (Ht_item*)table->items[index];
}

int ht_insert(HashTable* table, char* key, char* value) { //void bila
	// Create the item
	Ht_item* item = create_item(key, value);

	// Compute the index
	unsigned long index = hash_function(atoi(key));

	Ht_item* current_item = table->items[index];

	if (current_item == NULL) {
		// Key does not exist.
		if (table->count == table->size) {
			// Hash Table Full
			printf("Insert Error: Hash Table is full\n");
			// Remove the create item
			free_item(item);
			return -1;
		}

		// Insert directly
		table->items[index] = item;
		table->count++;
		return atoi(key);
	}

	else {
		// Scenario 1: We only need to update value
		if (strcmp(current_item->key, key) == 0) {
			free_item(item);
			int oldKey = atoi(key);
			oldKey++;
			char newKey[20];
			itoa(oldKey, newKey, 10);
			return ht_insert(table, newKey, value);
			//strcpy(table->items[index]->value, value); //Samo ovo ostaje i if i return
			//return;
		}

		else {
			// Scenario 2: Collision
			// We will handle case this a bit later
			//handle_collision(table, index, item);
			return -1;
		}
	}
}

void print_table(HashTable* table) {
	printf("\nHash Table\n-------------------\n");
	for (int i = 0; i < table->size; i++) {
		if (table->items[i]) {
			printf("Index:%d, Key:%s, Value:%s\n", i, table->items[i]->key, table->items[i]->value);
		}
	}
	printf("-------------------\n\n");
}


int ht_insert_auto_val(HashTable* table, char* key) {
	// Create the item
	int k = atoi(key);
	k = k + CAPACITY; // Uvecamo za velicinu hashListe, znaci 9001 za kom sa servisom 10001 sa klijentom (Ako je lista 1000)
	char value[20];
	itoa(k, value, 10);

	Ht_item* item = create_item(key, value);

	// Compute the index
	unsigned long index = hash_function(atoi(key));

	Ht_item* current_item = table->items[index];

	if (current_item == NULL) {
		// Key does not exist.
		if (table->count == table->size) {
			// Hash Table Full
			printf("Insert Error: Hash Table is full\n");
			// Remove the create item
			free_item(item);
			return -1;
		}

		// Insert directly
		table->items[index] = item;
		table->count++;
		return atoi(key);
	}

	else {
		// Scenario 1: We only need to update value
		if (strcmp(current_item->key, key) == 0) {
			free_item(item);
			int oldKey = atoi(key);
			oldKey++;
			char newKey[20];
			itoa(oldKey, newKey, 10);
			return ht_insert(table, newKey, value);
			//strcpy(table->items[index]->value, value); //Samo ovo ostaje i if i return
			//return;
		}

		else {
			// Scenario 2: Collision
			// We will handle case this a bit later
			//handle_collision(table, index, item);
			return -1;
		}
	}
}