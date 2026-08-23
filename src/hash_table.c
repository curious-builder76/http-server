

// Stolen from https://github.com/FiveThread/cerveur_fork/blob/master/src/hash_table.c


#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hash_table.h"


#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL
#define INIT_CAP 16

typedef struct
{
	const char *key;
	void *value;
}ht_entry;


struct ht
{
	ht_entry *entries;		//Hash slots.
	size_t capacity;			//size of _entries array.
	size_t length;				//number of items in hashtable.
};



ht *ht_create(void)
{
	//Allcolate space for hashtable struct.
	ht *table = malloc(sizeof(ht));
	if(table==NULL)
	{
		return NULL;
	}
	
	table->length = 0;
	table->capacity = INIT_CAP;
	
	//Allcolate space of entry buckets.
	table->entries = calloc(table->capacity, sizeof(ht_entry));
	if(table->entries == NULL)
	{
		free(table);
		return NULL;
	}
	return table;
}

void ht_destory(ht *table)
{
	//First free allocated keys.
	for(size_t i = 0; i < table->capacity; i++)
	{
		free((void*)table->entries[i].key);
	}
	
	//Then free entries array and table.
	free(table->entries);
	free(table);
}

static uint64_t hash_key(const char *key)
{
	uint64_t hash = FNV_OFFSET;
	for(const char *p = key; *p; p++)
	{
		hash ^= (uint64_t)(unsigned char)(*p);
		hash *= FNV_PRIME;
	}
	return hash;
}

static const char *ht_set_entry(ht_entry *entries, size_t capacity, const char *key, void *value, size_t *plength)
{
	//AND hash with capacity-1 to ensure its within entries array.
	uint64_t hash = hash_key(key);
	size_t index = (size_t)(hash & (uint64_t)(capacity-1));
	
	//Loop till we find an empty entry
	while(entries[index].key != NULL)
	{
		if(strcmp(key, entries[index].key) == 0)
		{
		//Found key update value
			entries[index].value = value;
			return entries[index].key;
		}
			//Key wanst in this slot move to next
		index++;
		if(index >= capacity)
		{
			//At end of entries array, wrap around
			index = 0;
		}
	}
	
	//Didnt find key, allocate if needed, then insert
	if(plength != NULL)
	{
		key = strdup(key);
		if(key == NULL)
		{
			return NULL;
		}
		(*plength)++;
	}
	entries[index].key = (char*)key;
	entries[index].value = value;
	return key;
}

static bool ht_expand(ht *table)
{
	//Allocate new entries array
	size_t new_cap = table->capacity * 2;
	if(new_cap < table->capacity)
	{
		return false;		//overflow
	}
	
	ht_entry *new_entries = calloc(new_cap, sizeof(ht_entry));
	if(new_entries == NULL)
	{
		return false;
	}
	
	//Iterate entries, move all non-empty ones to new table's entries
	for(size_t i =0; i < table->capacity; i++)
	{
		ht_entry entry = table->entries[i];
		if(entry.key != NULL)
		{
			ht_set_entry(new_entries, new_cap, entry.key, entry.value, NULL);
		}
	}
	
	//Free old entries
	free(table->entries);
	table->entries = new_entries;
	table->capacity = new_cap;
	return true;
}

void *ht_get(ht *table, const char *key)
{
	//AND hash with capacity-1 to ensure its within entries array.
	uint64_t hash = hash_key(key);
	size_t index = (size_t)(hash & (uint64_t)(table->capacity -1));
	
	//Loop till we find and empty entry;
	while(table->entries[index].key != NULL)
	{
		if(strcmp(key, table->entries[index].key) == 0)
		{
			//Found key return value
			return table->entries[index].value;
		}
		//If key wasnt found move to next
		index++;
		if(index >= table->capacity)
		{
			//At end of entries array wrap around
			index = 0;
		}
	}
	return NULL;
}

const char *ht_set(ht *table, const char *key, void *value)
{
	assert(value != NULL);
	if(value == NULL) return NULL;
	
	//If lengtyh will exceed half of current capacity, expand it
	if(table->length >= table->capacity / 2)
	{
		if(!ht_expand(table))
		{
			return NULL;
		}
	}
	
	return ht_set_entry(table->entries, table->capacity, key, value, &table->length);
}

size_t ht_length(ht *table)
{
	return table->length;
}

hti ht_iterator(ht *table)
{
	hti it;
	it._table = table;
	it._index = 0;
	return it;
}

bool ht_next(hti *it)
{
	//Loop till we hit end of entries array
	ht *table = it->_table;
	while (it->_index < table->capacity)
	{
		size_t i = it->_index;
		it->_index++;
		if(table->entries[i].key != NULL)
		{
			//Found next non-empty item, update iterator key and value
			ht_entry entry = table->entries[i];
			it->key = entry.key;
			it->value = entry.value;
			return true;
		}
	}
	return false;
}
