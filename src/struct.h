// Copyright (c) 2018 Roland Bernard

#ifndef __STRUCT_H__
#define __STRUCT_H__

#include "./types.h"
#include "./operation.h"
#include "./variabletable.h"

typedef environment_t struct_t;

struct_t* struct_create();
void struct_exec(struct_t* stc, operation_t* op);
object_t** struct_result(struct_t* stc, operation_t* op);
object_t*** struct_var(struct_t* stc, operation_t* op);
void struct_free(struct_t* stc);
id_t struct_id(struct_t* stc);
bool_t struct_equ(struct_t* s1, struct_t* s2);

#endif