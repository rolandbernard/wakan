// Copyright (c) 2018-2019 Roland Bernard

#include "./function.h"
#include "./environment.h"
#include "./object.h"
#include "./prime.h"
#include "./langallocator.h"
#include "./error.h"


function_t* function_create(operation_t* par, operation_t* func) {
    function_t* ret = (function_t*)_alloc(sizeof(function_t));

    ret->freeable = true;
    ret->parameter = operation_copy(par);
    ret->function = operation_copy(func);

    return ret;
}

function_t* function_create_reference(operation_t* par, operation_t* func) {
    function_t* ret = (function_t*)_alloc(sizeof(function_t));

    ret->freeable = false;
    ret->parameter = par;
    ret->function = func;

    return ret;
}

void* function_exec(function_t* func, object_t** par, environment_t* env) {
    void* ret = NULL;

    if(check_for_stackoverflow()) {
        error("Runtime error: Stack overflow.");
        ret = RET_ERROR;
    } else
        if(func != NULL) {
            environment_add_scope(env);
            size_t prev_limit = env->local_mode_limit;
            environment_set_local_mode(env, env->count-1);

            string_t* func_self_name = string_create("func_self");
            environment_write(env, func_self_name, object_create_function(function_create_reference(func->parameter, func->function)));

            object_t*** par_loc_list = operation_var(func->parameter, env);

            if(par != NULL && par_loc_list != NULL) {
                int i;
                for(i = 0; par_loc_list[i] != NULL && par[i] != NULL; i++) {
                    if(par_loc_list[i] == OBJECT_LIST_OPENED) {
                        object_t** obj = par_loc_list[i+1];
                        object_dereference(*obj);

                        size_t length_left = 0;
                        while(par[i + length_left] != NULL) length_left++;

                        list_t* list = list_create_null(length_left);
                        for(int j = 0; j < length_left; j++) {
                            list->data[j] = par[i+j];
                            object_reference(list->data[j]);
                        }
                        *obj = object_create_list(list);
                        object_reference(*obj);
                        break;
                    } else {
                        object_dereference(*(par_loc_list[i]));
                        *(par_loc_list[i]) = par[i];
                        object_reference(*(par_loc_list[i]));
                    }
                }
                if(par[i] != NULL && par_loc_list[i] == NULL) {
                    error("Runtime error: Too many arguments to function.");
                    ret = RET_ERROR;
                }
                _free(par_loc_list);
            }
            if(par != NULL && par_loc_list == NULL){
                error("Runtime error: Too many arguments to function.");
                ret = RET_ERROR;
            } else if(ret != RET_ERROR) {
                if(operation_exec(func->function, env) == RET_ERROR)
                    ret = RET_ERROR;
            }

            environment_del(env, func_self_name);
            string_free(func_self_name);

            environment_set_local_mode(env, prev_limit);
            environment_remove_scope(env);
        }

    return ret;
}


object_t** function_result(function_t* func, object_t** par, environment_t* env) {
    object_t** ret = NULL;

    if(check_for_stackoverflow()) {
        error("Runtime error: Stack overflow.");
        ret = RET_ERROR;
    } else
        if(func != NULL) {
            environment_add_scope(env);
            size_t prev_limit = env->local_mode_limit;
            environment_set_local_mode(env, env->count-1);

            string_t* func_self_name = string_create("func_self");
            environment_write(env, func_self_name, object_create_function(function_create_reference(func->parameter, func->function)));

            object_t*** par_loc_list = operation_var(func->parameter, env);

            if(par != NULL && par_loc_list != NULL) {
                int i;
                for(i = 0; par_loc_list[i] != NULL && par[i] != NULL; i++) {
                    if(par_loc_list[i] == OBJECT_LIST_OPENED) {
                        object_t** obj = par_loc_list[i+1];
                        object_dereference(*obj);

                        size_t length_left = 0;
                        while(par[i + length_left] != NULL) length_left++;

                        list_t* list = list_create_null(length_left);
                        for(int j = 0; j < length_left; j++) {
                            list->data[j] = par[i+j];
                            object_reference(list->data[j]);
                        }
                        *obj = object_create_list(list);
                        object_reference(*obj);
                        break;
                    } else {
                        object_dereference(*(par_loc_list[i]));
                        *(par_loc_list[i]) = par[i];
                        object_reference(*(par_loc_list[i]));
                    }
                }
                if(par[i] != NULL && par_loc_list[i] == NULL) {
                    error("Runtime error: Too many arguments to function.");
                    ret = RET_ERROR;
                }
                _free(par_loc_list);
            }

            if(par != NULL && par_loc_list == NULL){
                error("Runtime error: Too many arguments to function.");
                ret = RET_ERROR;
            } else if(ret != RET_ERROR) {
                ret = operation_result(func->function, env);
            }

            environment_del(env, func_self_name);
            string_free(func_self_name);

            environment_set_local_mode(env, prev_limit);
            environment_remove_scope(env);
        }

    return ret;
}

void function_free(function_t* func) {
    if(func != NULL) {
        if(func->freeable) {
            operation_free(func->function);
            operation_free(func->parameter);
        }
        _free(func);
    }
}

id_t function_id(function_t* func) {
    return SMALL_PRIME_2 * operation_id(func->function) + operation_id(func->parameter);
}

bool_t function_equ(function_t* f1, function_t* f2) {
    return operation_equ(f1->function, f2->function) && operation_equ(f1->parameter, f2->parameter);
}
