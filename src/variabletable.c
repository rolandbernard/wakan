// Copyright (c) 2018-2019 Roland Bernard

#include <math.h>

#include "./prime.h"
#include "./object.h"
#include "./variabletable.h"
#include "./langallocator.h"

// TODO: Improve collision handling (Double hashing)
upos_t variabletable_find(variabletable_t* tbl, string_t* name) {
    if(tbl != NULL) {
        upos_t index = string_id(name) % tbl->size;

        while(tbl->data[index] != NULL && !string_equ(name, tbl->data[index]->name))
            index = (index + 1) % tbl->size;

        return index;
    } else 
        return ~0;
}

void variabletable_resize(variabletable_t* tbl, size_t size) {
    variabletable_t* temp_data = (variabletable_t*)_alloc(sizeof(variabletable_t));

    temp_data->size = next_prime(size);
    temp_data->data = (bucket_element_t**)_alloc(sizeof(bucket_element_t*)*temp_data->size);
    for(int i = 0; i < temp_data->size; i++)
        temp_data->data[i] = NULL;

    for(int i = 0; i < tbl->size; i++)
        if(tbl->data[i] != NULL)
            temp_data->data[variabletable_find(temp_data, tbl->data[i]->name)] = tbl->data[i];

    _free(tbl->data);
    tbl->size = temp_data->size;
    tbl->data = temp_data->data;
    _free(temp_data);
}

void variabletable_extend(variabletable_t* tbl) {
    if(tbl != NULL)
        variabletable_resize(tbl, tbl->size * 2);
}

void variabletable_shrink(variabletable_t* tbl) {
    if(tbl != NULL)
        variabletable_resize(tbl, tbl->size / 2);
}

void variabletable_check_size(variabletable_t* tbl) {
    if(tbl != NULL) {
        if(tbl->count * 10 / tbl->size >= 7)
            variabletable_extend(tbl);
        else if(tbl->count * 10 / tbl->size == 0 && tbl->count > 1)
            variabletable_shrink(tbl);
    }
}

variabletable_t* variabletable_create() {
    variabletable_t* ret = (variabletable_t*)_alloc(sizeof(variabletable_t));
    size_t real_size = next_prime(DEFAULT_START_SIZE);

    ret->count = 0;
    ret->data = (bucket_element_t**)_alloc(sizeof(bucket_element_t*) * real_size);
    for(int i = 0; i < real_size; i++)
        ret->data[i] = NULL;
    ret->size = real_size;

    return ret;
}

void variabletable_make(variabletable_t* tbl, string_t* name) {
    if(tbl != NULL) {
        upos_t index = variabletable_find(tbl, name);
        if(tbl->data[index] == NULL) {
            tbl->data[index] = (bucket_element_t*)_alloc(sizeof(bucket_element_t));
            tbl->data[index]->name = string_copy(name);
            tbl->data[index]->value = object_create_none();
            object_reference(tbl->data[index]->value);
            tbl->count++;
            variabletable_check_size(tbl);
        }
    }
}

void variabletable_del(variabletable_t* tbl, string_t* name) {
    if(tbl != NULL) {
        upos_t index = variabletable_find(tbl, name);
        if(tbl->data[index] != NULL) {
            string_free(tbl->data[index]->name);
            object_dereference(tbl->data[index]->value);
            _free(tbl->data[index]);
            tbl->data[index] = NULL;
            tbl->count--;
            variabletable_check_size(tbl);
        }
    }
}

void variabletable_write(variabletable_t* tbl, string_t* name, object_t* data) {
    if(tbl != NULL) {
        upos_t index = variabletable_find(tbl, name);
        if(tbl->data[index] == NULL) {
            tbl->data[index] = (bucket_element_t*)_alloc(sizeof(bucket_element_t));
            tbl->data[index]->name = string_copy(name);
            tbl->data[index]->value = NULL;
            tbl->count++;
            variabletable_check_size(tbl);
        }
        index = variabletable_find(tbl, name);
        object_dereference(tbl->data[index]->value);
        tbl->data[index]->value = data;
        object_reference(tbl->data[index]->value);
    }
}

object_t* variabletable_get(variabletable_t* tbl, string_t* name) {
    if(tbl != NULL) {
        upos_t index = variabletable_find(tbl, name);
        if(tbl->data[index] == NULL)
            return NULL;
        else
            return tbl->data[index]->value;
    } else
        return NULL;
}

object_t** variabletable_get_loc(variabletable_t* tbl, string_t* name) {
    if(tbl != NULL) {
        upos_t index = variabletable_find(tbl, name);
        if(tbl->data[index] == NULL) {
            tbl->data[index] = (bucket_element_t*)_alloc(sizeof(bucket_element_t));
            tbl->data[index]->name = string_copy(name);
            tbl->data[index]->value = object_create_none();
            object_reference(tbl->data[index]->value);
            tbl->count++;
            variabletable_check_size(tbl);
        }
        index = variabletable_find(tbl, name);
        return &(tbl->data[index]->value);
    } else
        return NULL;
}

bool_t variabletable_exists(variabletable_t* tbl, string_t* name) {
    return tbl->data[variabletable_find(tbl, name)] != NULL;
}

id_t variabletable_id(variabletable_t* tbl) {
    long hash = 0;
    int count_hashed = 0;
    for (int i = 0; i < tbl->size; i++) 
        if(tbl->data[i] != NULL) {
            count_hashed++;
            hash += (long)pow(SMALL_PRIME_4, tbl->count - count_hashed) * (string_id(tbl->data[i]->name) + object_id(tbl->data[i]->value) * BIG_PRIME_3);
        }
    return (id_t)hash;
}

bool_t variabletable_equ(variabletable_t* t1, variabletable_t* t2) {
    if(t1->count != t2->count)
        return false;
    
    for(int i = 0; i < t1->size; i++)
        if(t1->data[i] != NULL)
            if(!object_equ(t1->data[i]->value, variabletable_get(t2, t1->data[i]->name)))
                return false;

    return true;
}

void variabletable_free(variabletable_t* tbl) {
    if(tbl != NULL) {
        if(tbl->data != NULL) {
            for(int i = 0; i < tbl->size; i++)
                if(tbl->data[i] != NULL) {
                    string_free(tbl->data[i]->name);
                    object_dereference(tbl->data[i]->value);
                    _free(tbl->data[i]);
                }
            _free(tbl->data);
        }
        _free(tbl);
    }
}