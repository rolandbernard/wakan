// Copyright (c) 2018-2019 Roland Bernard

#include "./struct.h"
#include "./error.h"

struct_t* struct_create() {
    return environment_create();
}

void* struct_exec(struct_t* stc, operation_t* op) {
    void* ret = NULL;

    if(check_for_stackoverflow()) {
        error("Runtime error: Stack overflow.");
        ret = RET_ERROR;
    } else {
        int prev_local_limit = stc->local_mode_limit;
        environment_set_local_mode(stc, 0);
        ret = operation_exec(op, stc);
        environment_set_local_mode(stc, prev_local_limit);
    }    

    return ret;
}

object_t** struct_result(struct_t* stc, operation_t* op) {
    object_t** ret;

    if(check_for_stackoverflow()) {
        error("Runtime error: Stack overflow.");
        ret = RET_ERROR;
    } else {
        int prev_local_limit = stc->local_mode_limit;
        environment_set_local_mode(stc, 0);
        ret = operation_result(op, stc);
        environment_set_local_mode(stc, prev_local_limit);
    }

    return ret;
}

object_t*** struct_var(struct_t* stc, operation_t* op) {
    object_t*** ret;

    if(check_for_stackoverflow()) {
        error("Runtime error: Stack overflow.");
        ret = RET_ERROR;
    } else {
        int prev_local_limit = stc->local_mode_limit;
        environment_set_local_mode(stc, 0);
        ret = operation_var(op, stc);
        environment_set_local_mode(stc, prev_local_limit);
    }

    return ret;
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