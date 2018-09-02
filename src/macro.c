// Copyright (c) 2018 Roland Bernard

#include "./macro.h"

macro_t* macro_create(operation_t* mac) {
	return mac;
}

void macro_exec(macro_t* mac, environment_t* env) {
	operation_exec(mac, env);
}

object_t** macro_result(macro_t* mac, environment_t* env) {
	return operation_result(mac, env);
}

object_t*** macro_var(macro_t* mac, environment_t* env) {
	return operation_var(mac, env);
}

void macro_free(macro_t* mac) {
	// Operations in Objects don't have to be freed since they are part of the tree
	//operation_free(mac);
}

id_t macro_id(macro_t* mac) {
	return operation_id(mac);
}

bool_t macro_equ(macro_t* m1, macro_t* m2) {
	return operation_equ(m1, m2);
}