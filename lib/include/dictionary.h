// Copyright (c) 2018 Roland Bernard

#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#define DEFAULT_START_SIZE 11

#include "./types.h"
#include "./pair.h"
#include "./bool.h"

typedef struct dictionary_s {
	size_t size;
	size_t count;
	pair_t** data;
} dictionary_t;

dictionary_t* dictionary_create();
dictionary_t* dictionary_create_sized(size_t size); // This will use the next bigger prime.
dictionary_t* dictionary_copy(dictionary_t* dic);
void dictionary_resize(dictionary_t* dic, size_t size); // This will use the next bigger prime.
object_t* dictionary_get(dictionary_t* dic, object_t* key);
object_t** dictionary_get_loc(dictionary_t* dic, object_t* key);
void dictionary_put_pair(dictionary_t* dic, pair_t* pair); // If this function is used the pair will be freed together with the dictionary
void dictionary_put(dictionary_t* dic, object_t* key, object_t* value); // Value and key will be dereferenced when freeing the dictionary or deleting the entry
void dictionary_del(dictionary_t* dic, object_t* key);
void dictionary_free(dictionary_t* dic);
bool_t dictionary_equ(dictionary_t* d1, dictionary_t* d2); // Two dictionaries are equal if they have the same key-value-pairs regardless of size
id_t dictionary_id(dictionary_t* dic);

#endif