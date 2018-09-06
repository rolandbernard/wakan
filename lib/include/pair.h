// Copyright (c) 2018 Roland Bernard

#ifndef __PAIR_H__
#define __PAIR_H__

#include "./types.h"
#include "./bool.h"

typedef struct pair_s {
	object_t* key;
	object_t* value;
} pair_t;

pair_t* pair_create(object_t* k, object_t* v);
pair_t* pair_copy(pair_t* pair);
id_t pair_id(pair_t* pair);
bool_t pair_equ(pair_t* p1, pair_t* p2);
void pair_free(pair_t* pair);

#endif