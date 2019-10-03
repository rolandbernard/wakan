// Copyright (c) 2018-2019 Roland Bernard

#include "./pair.h"
#include "./object.h"
#include "./langallocator.h"
#include "./prime.h"

pair_t* pair_create(object_t* k, object_t* v) {
    pair_t* ret = (pair_t*)_alloc(sizeof(pair_t));
    ret->key = k;
    object_reference(k);
    ret->value = v;
    object_reference(v);
    return ret;
}

pair_t* pair_copy(pair_t* pair) {
    pair_t* ret = NULL;

    if(pair != NULL) {
        ret = (pair_t*)_alloc(sizeof(pair_t));
        ret->key = pair->key;
        object_reference(ret->key);
        ret->value = pair->value;
        object_reference(ret->value);
    }

    return ret;
}

// TODO:
id_t pair_id(pair_t* pair) {
    if(pair != NULL)
        return SMALL_PRIME_2 * object_id(pair->key) + object_id(pair->value);
    else
        return 0;
}

bool_t pair_equ(pair_t* p1, pair_t* p2) {
    if(p1 != NULL && p2 != NULL)
        return object_equ(p1->key, p2->key) && object_equ(p1->value, p2->value);
    else
        return p1 == p2;
}

void pair_free(pair_t* pair) {
    if(pair != NULL) {
        object_dereference(pair->key);
        object_dereference(pair->value);
        _free(pair);
    }
}