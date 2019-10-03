// Copyright (c) 2018-2019 Roland Bernard

#ifndef __VARIABLETABLE_H__
#define __VARIABLETABLE_H__

#define DEFAULT_START_SIZE 11

#include "./types.h"
#include "./bool.h"
#include "./string.h"

typedef struct variabletable_s {
    struct bucket_element_s {
        string_t* name;
        object_t* value;
    }** data;
    size_t size;
    size_t count; 
} variabletable_t;
typedef struct bucket_element_s bucket_element_t;

variabletable_t* variabletable_create();
void variabletable_make(variabletable_t* tbl, string_t* name);
void variabletable_del(variabletable_t* tbl, string_t* name);
void variabletable_write(variabletable_t* tbl, string_t* name, object_t* data);
object_t* variabletable_get(variabletable_t* tbl, string_t* name);
object_t** variabletable_get_loc(variabletable_t* tbl, string_t* name);
bool_t variabletable_exists(variabletable_t* tbl, string_t* name);
id_t variabletable_id(variabletable_t* tbl);
bool_t variabletable_equ(variabletable_t* t1, variabletable_t* t2);
void variabletable_free(variabletable_t* tbl);

#endif