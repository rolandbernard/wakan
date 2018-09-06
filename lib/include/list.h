// Copyright (c) 2018 Roland Bernard

#ifndef __LIST_H__
#define __LIST_H__

#include "./types.h"
#include "./bool.h"

typedef struct list_s {
	object_t** data;
	size_t size;
} list_t;

list_t* list_create_empty();
list_t* list_create_null(size_t size);
list_t* list_copy(list_t* list);
list_t* list_add(list_t* l1, list_t* l2);
list_t* list_mul(list_t* list, size_t n);
list_t* list_range(list_t* list, pos_t pos, pos_t n);
pos_t list_find(list_t* list, object_t* obj);
object_t* list_get(list_t* list, pos_t pos);
object_t** list_get_loc(list_t* list, pos_t pos); 
size_t list_size(list_t* list);
id_t list_id(list_t* list);
bool_t list_equ(list_t* l1, list_t* l2);
void list_append(list_t* list, object_t* obj);
void list_free(list_t* list);

#endif