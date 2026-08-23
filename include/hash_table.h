

// Stolen from: 
// https://github.com/FiveThread/cerveur_fork/blob/master/include/hash_table.h

#ifndef _HASH_TABLE_H
#define _HASH_TABLE_H

#include <stdbool.h>
#include <stddef.h>


typedef struct ht ht;

//Create hashtable and return pointer to it, or NULL if out of memory
ht *ht_create(void);

//Free memory allocated for hash table
void ht_destory(ht *table); 

//Get item with key from hashtable and return value. return null if not found
void *ht_get(ht *table, const char *key);

//Set item with givien key to value.
//If not present in table key is tranfered
//newly allocated memory
//Return address of key or NULL if out of memory
const char *ht_set(ht *table, const char *key, void *value);

//Return number of iteams 
size_t ht_length(ht *table);

typedef struct
{
	const char *key;	//current key
	void *value;			//current vlaue
	
	//not intended for use
	ht *_table;
	size_t _index;
}hti;

//Returns new hash table
hti ht_iterator(ht *table);

//Move iterator to next item in hashtable
bool ht_next(hti *it);


#endif
