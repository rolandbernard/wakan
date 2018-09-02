// Copyright (c) 2018 Roland Bernard

#include "./struct.h"

struct_t* struct_create() {
	return environment_create();
}

void struct_exec(struct_t* stc, operation_t* op) {
	operation_exec(op, stc);
}

object_t** struct_result(struct_t* stc, operation_t* op) {
	return operation_result(op, stc);
}

object_t*** struct_var(struct_t* stc, operation_t* op) {
	return operation_var(op, stc);
}

void struct_free(struct_t* stc) {
	environment_free(stc);
}

id_t struct_id(struct_t* stc) {
	return environment_id(stc);
}

bool_t struct_equ(struct_t* s1, struct_t* s2) {
	return environment_equ(s1, s2);
}