#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include "./types.h"
#include "./operation.h"

typedef struct function_s {
	operation_t* parameter;
	operation_t* function;
} function_t;

function_t* function_create(operation_t* par, operation_t* func);
void function_exec(function_t* func, object_t** par, environment_t* env);
object_t** function_result(function_t* func, object_t** par, environment_t* env);
void function_free(function_t* func);
id_t function_id(function_t* func);
bool_t function_equ(function_t* f1, function_t* f2);

#endif