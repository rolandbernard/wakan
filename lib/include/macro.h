// Copyright (c) 2018-2019 Roland Bernard

#ifndef __MACRO_H__
#define __MACRO_H__

#include "./types.h"
#include "./operation.h"

typedef operation_t macro_t;

macro_t* macro_create(operation_t* mac);
void* macro_exec(macro_t* mac, environment_t* env);
object_t** macro_result(macro_t* mac, environment_t* env);
object_t*** macro_var(macro_t* mac, environment_t* env);
void macro_free(macro_t* mac);
id_t macro_id(macro_t* mac);
bool_t macro_equ(macro_t* m1, macro_t* m2);

#endif