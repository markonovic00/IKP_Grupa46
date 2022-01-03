#ifndef HASHTABLE_DOT_H
#define HASHTABLE_DOT_H
#define CAPACITY 1000 //Size of the Hash Table (Maksimalan broj aktivnih dostavljaca) 

typedef struct Ht_item Ht_item;
struct  Ht_item
{
    char* key; //Port za komunikaciju sa servisom
    char* value; //Port za komunikaciju sa klijentom
};

typedef struct HashTable HashTable;
struct HashTable {
    Ht_item** items;
    int size;
    int count;
};

#include <corecrt_malloc.h>
#include <string.h>

unsigned long hash_function(unsigned i);
unsigned long hash_function(char* str);

Ht_item* create_item(char* key, char* value);
HashTable* create_table(int size);

void free_item(Ht_item* item);
void free_table(HashTable* table);

char* ht_search(HashTable* table, char* key);

int ht_insert(HashTable* table, char* key, char* value);
int ht_insert_auto_val(HashTable* table, char* key);

void print_table(HashTable* table);
Ht_item* ht_get_item_pointer(HashTable* table, char* key);

int ht_get_empty_index(HashTable* table);

#endif // !HASHTABLE_DOT_H




