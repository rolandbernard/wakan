// Copyright (c) 2018 Roland Bernard

#include <math.h>

#include "./dictionary.h"
#include "./langallocator.h"
#include "./prime.h"
#include "./object.h"

void dictionary_extend(dictionary_t* dic) {
	if(dic != NULL)
		dictionary_resize(dic, dic->size * 2);
}

void dictionary_shrink(dictionary_t* dic) {
	if(dic != NULL)
		dictionary_resize(dic, dic->size / 2);
}

void dictionary_check_size(dictionary_t* dic) {
	if(dic != NULL) {
		if(dic->count * 10 / dic->size >= 7)
			dictionary_extend(dic);
		else if(dic->count * 10 / dic->size == 0 && dic->count > 1)
			dictionary_shrink(dic);
	}
}

// TODO: Improve collision handling (Double hashing)
upos_t dictionary_find(dictionary_t* dic, object_t* key) {
	if(dic != NULL) {
		upos_t index = object_id(key) % dic->size;

		while(dic->data[index] != NULL && !object_equ(key, dic->data[index]->key))
			index = (index + 1) % dic->size;

		return index;
	} else 
		return ~0;
}


dictionary_t* dictionary_create() {
	return dictionary_create_sized(DEFAULT_START_SIZE);
}

dictionary_t* dictionary_create_sized(size_t size) {
	dictionary_t* ret = (dictionary_t*)_alloc(sizeof(dictionary_t));
	size_t real_size = next_prime(size);

	ret->count = 0;
	ret->data = (pair_t**)_alloc(sizeof(pair_t*) * real_size);
	for(int i = 0; i < real_size; i++)
		ret->data[i] = NULL;
	ret->size = real_size;

	return ret;
}

dictionary_t* dictionary_copy(dictionary_t* dic) {
	dictionary_t* ret = NULL;

	if(dic != NULL) {
		ret = (dictionary_t*)_alloc(sizeof(dictionary_t));
		ret->data = (pair_t**)_alloc(sizeof(pair_t*) * dic->size);
		ret->count = dic->count;
		for(ret->size = 0; ret->size < dic->size; ret->size++)
			ret->data[ret->size] = pair_copy(dic->data[ret->size]);
	}

	return ret;
}

void dictionary_resize(dictionary_t* dic, size_t size) {
	if(dic != NULL) {
		dictionary_t* temp_new_dic = dictionary_create_sized(size);

		for(int i = 0; i < dic->count; i++)
			if(dic->data[i] != NULL)
				dictionary_put_pair(temp_new_dic, dic->data[i]);

		_free(dic->data);
		dic->data = temp_new_dic->data;
		dic->size = temp_new_dic->size;
		_free(temp_new_dic);
	}
}

void dictionary_put_pair(dictionary_t* dic, pair_t* pair) {
	if(dic != NULL) {
		upos_t index = dictionary_find(dic, pair->key);
		if(dic->data[index] != NULL)
			pair_free(dic->data[index]);
		else 
			dic->count++;
		dic->data[index] = pair;
		dictionary_check_size(dic);
	}
}

void dictionary_put(dictionary_t* dic, object_t* key, object_t* value) {
	if(dic != NULL) {
		upos_t index = dictionary_find(dic, key);
		if(dic->data[index] == NULL) {
			dic->data[index] = pair_create(key, value);
			dic->count++;
		} else {
			object_dereference(dic->data[index]->value);
			dic->data[index]->value = value;
			object_reference(value);
		}
		dictionary_check_size(dic);
	}
}

object_t* dictionary_get(dictionary_t* dic, object_t* key) {
	if(dic != NULL) {
		upos_t index = dictionary_find(dic, key);
		if(dic->data[index] != NULL)
			return dic->data[index]->value;
		else
			return NULL;
	}
	else
		return NULL;
}

object_t** dictionary_get_loc(dictionary_t* dic, object_t* key) {
	if(dic != NULL) {
		upos_t index = dictionary_find(dic, key);
		if(dic->data[index] != NULL)
			return &(dic->data[index]->value);
		else {
			dic->data[index] = pair_create(key, object_create_none());
			dic->count++;
			dictionary_check_size(dic);
			return &(dic->data[dictionary_find(dic, key)]->value);
		}
	}
	else
		return NULL;
}

void dictionary_del(dictionary_t* dic, object_t* key) {
	if(dic != NULL) {
		upos_t index = dictionary_find(dic, key);
		if(dic->data[index] != NULL) {
			pair_free(dic->data[index]);
			dic->count--;
		}
		dictionary_check_size(dic);
	}
}

void dictionary_free(dictionary_t* dic) {
	if(dic != NULL) {
		if(dic->data != NULL) {
			for(int i = 0; i < dic->size; i++)
				if(dic->data[i] != NULL)
					pair_free(dic->data[i]);
			
			_free(dic->data);
		}
		_free(dic);
	}
}

bool_t dictionary_equ(dictionary_t* d1, dictionary_t* d2) {
	if(d1->count != d2->count)
		return false;
	
	for(int i = 0; i < d1->size; i++)
		if(d1->data[i] != NULL)
			if(!object_equ(d1->data[i]->value, dictionary_get(d2, d1->data[i]->key)))
				return false;

	return true;
}

id_t dictionary_id(dictionary_t* dic) {
	long hash = 0;
	int count_hashed = 0;
    for (int i = 0; i < dic->size; i++) 
		if(dic->data[i] != NULL) {
			count_hashed++;
			hash += (long)pow(SMALL_PRIME_4, dic->count - count_hashed) * pair_id(dic->data[i]);
    	}
    return (id_t)hash;
}