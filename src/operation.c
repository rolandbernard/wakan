// Copyright (c) 2018-2019 Roland Bernard

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "./operation.h"
#include "./object.h"
#include "./langallocator.h"
#include "./prime.h"
#include "./error.h"
#include "./program.h"

#define TMP_STR_MAX 1<<12

static void (*old_error_handler)(const char* msg) = NULL;
static bool_t import_error_flag = false;

void import_error_handler(const char* msg) {
    old_error_handler(msg);
    import_error_flag = true;
}

operation_t* operation_create() {
    return (operation_t*)_alloc(sizeof(operation_t));
}

operation_t* operation_create_NOOP() {
    operation_t* ret = (operation_t*)_alloc(sizeof(operation_t));

    ret->type = OPERATION_TYPE_NOOP;

    return ret;
}

void* operation_exec(operation_t* op, environment_t* env) {
    void* ret = NULL;

    if(check_for_stackoverflow()) {
        error("Runtime error: Stack overflow.");
        ret = RET_ERROR;
    } else
        if(op != NULL) {
            switch(op->type) {
                case OPERATION_TYPE_NOOP:
                case OPERATION_TYPE_NOOP_BRAC:
                case OPERATION_TYPE_NOOP_EMP_REC:
                case OPERATION_TYPE_NOOP_O_LIST_DEADEND:
                case OPERATION_TYPE_NOOP_PROC_DEADEND:
                case OPERATION_TYPE_NOOP_PLUS:
                case OPERATION_TYPE_NOOP_EMP_CUR: break;
                case OPERATION_TYPE_NONE: break;
                case OPERATION_TYPE_NUM: break;
                case OPERATION_TYPE_STR: break;
                case OPERATION_TYPE_VAR: {
                    environment_make(env, op->data.str);
                    object_t* var = environment_get(env, op->data.str);
                    if(var->type == OBJECT_TYPE_MACRO)
                        if(macro_exec(var->data.mac, env) == RET_ERROR)
                            ret = RET_ERROR;
                } break;
                case OPERATION_TYPE_BOOL: break;
                case OPERATION_TYPE_PAIR:
                    if (operation_exec(op->data.operations[0], env) == RET_ERROR ||
                        operation_exec(op->data.operations[1], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_FUNCTION: break;
                case OPERATION_TYPE_MACRO: break;
                case OPERATION_TYPE_ASSIGN: {
                    object_t**    assign_val = operation_result(op->data.operations[1], env);
                    object_t*** assign_loc = operation_var(op->data.operations[0], env);

                    if(assign_loc == NULL || assign_val == NULL) {
                        error("Runtime error: Assignment NULL error.");
                        ret = RET_ERROR;
                    } else if(assign_loc == RET_ERROR || assign_val == RET_ERROR) {
                        ret =  RET_ERROR;
                    } else
                        for(int i = 0; assign_loc[i] != NULL && assign_val[i] != NULL; i++) {
                            if(assign_loc[i] == OBJECT_LIST_OPENED) {
                                object_t** obj = assign_loc[i+1];
                                object_dereference(*obj);

                                size_t length_left = 0;
                                while(assign_val[i + length_left] != NULL) length_left++;

                                list_t* list = list_create_null(length_left);
                                for(int j = 0; j < length_left; j++) {
                                    list->data[j] = assign_val[i+j];
                                    object_reference(list->data[j]);
                                }
                                *obj = object_create_list(list);
                                object_reference(*obj);
                                break;
                            } else {
                                object_dereference(*(assign_loc[i]));
                                *(assign_loc[i]) = assign_val[i];
                                object_reference(*(assign_loc[i]));
                            }
                        }

                    if(assign_loc != RET_ERROR && assign_loc != NULL) {
                        _free(assign_loc);
                    }
                    if(assign_val != RET_ERROR && assign_val != NULL) {
                        for(int i = 0; assign_val[i] != NULL; i++)
                            object_dereference(assign_val[i]);
                        _free(assign_val);
                    }
                } break;
                case OPERATION_TYPE_PROC:
                case OPERATION_TYPE_PROC_IMP:
                    for(int i = 0; ret != RET_ERROR && op->data.operations[i] != NULL; i++)
                        if(operation_exec(op->data.operations[i], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_STRUCT:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_O_LIST:
                    for(int i = 0; op->data.operations[i] != NULL; i++)
                        if(operation_exec(op->data.operations[i], env))
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_LIST:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_INDEX:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR ||
                        operation_exec(op->data.operations[1], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_EXEC: {
                    object_t** function = operation_result(op->data.operations[0], env);
                    object_t** parameter = operation_result(op->data.operations[1], env);

                    if(function == NULL) {
                        error("Runtime error: Function NULL error.");
                        ret = RET_ERROR;
                    } else if(function == RET_ERROR || parameter == RET_ERROR) {
                        ret = RET_ERROR;
                    } else
                        for(int i = 0; ret != RET_ERROR && function[i] != NULL; i++)
                            if(function[i]->type != OBJECT_TYPE_FUNCTION) {
                                error("Runtime error: Function type error.");
                                ret = RET_ERROR;
                            } else
                                if(function_exec(function[i]->data.func, parameter, env) == RET_ERROR)
                                    ret = RET_ERROR;


                    if(function != RET_ERROR && function != NULL) {
                        for(int i = 0; function[i] != NULL; i++)
                            object_dereference(function[i]);
                        _free(function);
                    }
                    if(parameter != RET_ERROR && parameter != NULL) {
                        for(int i = 0; parameter[i] != NULL; i++)
                            object_dereference(parameter[i]);
                        _free(parameter);
                    }
                } break;
                case OPERATION_TYPE_TO_NUM:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_TO_BOOL:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_TO_ASCII:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_TO_STR:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_READ: {
                    char temp_str[TMP_STR_MAX];
                    fgets(temp_str, TMP_STR_MAX, stdin);
                } break;
                case OPERATION_TYPE_WRITE: {
                    object_t** data = operation_result(op->data.operations[0], env);

                    if(data == NULL) {
                        error("Runtime error: Write NULL error.");
                        ret = RET_ERROR;
                    } else if(data == RET_ERROR)
                        ret = RET_ERROR;
                    else
                        for(int i = 0; data[i] != NULL; i++)
                            print_object(data[i]);

                    if(data != RET_ERROR && data != NULL) {
                        for(int i = 0; data[i] != NULL; i++)
                            object_dereference(data[i]);
                        _free(data);
                    }
                } break;
                case OPERATION_TYPE_ADD:
                    for(int i = 0; op->data.operations[i] != NULL; i++)
                        if(operation_exec(op->data.operations[i], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_SUB:
                    for(int i = 0; op->data.operations[i] != NULL; i++)
                        if(operation_exec(op->data.operations[i], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_MUL:
                    for(int i = 0; op->data.operations[i] != NULL; i++)
                        if(operation_exec(op->data.operations[i], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_DIV:
                    for(int i = 0; op->data.operations[i] != NULL; i++)
                        if(operation_exec(op->data.operations[i], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_MOD:
                    for(int i = 0; op->data.operations[i] != NULL; i++)
                        if(operation_exec(op->data.operations[i], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_NEG:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_POW:
                    for(int i = 0; op->data.operations[i] != NULL; i++)
                        if(operation_exec(op->data.operations[i], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_AND:
                    for(int i = 0; op->data.operations[i] != NULL; i++)
                        if(operation_exec(op->data.operations[i], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_OR:
                    for(int i = 0; op->data.operations[i] != NULL; i++)
                        if(operation_exec(op->data.operations[i], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_XOR:
                    for(int i = 0; op->data.operations[i] != NULL; i++)
                        if(operation_exec(op->data.operations[i], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_NOT:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_SQRT:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_CBRT:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_SIN:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_COS:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_TAN:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_ASIN:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_ACOS:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_ATAN:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_SINH:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_COSH:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_TANH:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_ASINH:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_ACOSH:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_ATANH:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_TRUNC:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_FLOOR:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_CEIL:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_ROUND:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_RAND:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_LEN:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_EQU:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR ||
                        operation_exec(op->data.operations[1], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_GEQ:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR ||
                        operation_exec(op->data.operations[1], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_LEQ:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR ||
                        operation_exec(op->data.operations[1], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_GTR:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR ||
                        operation_exec(op->data.operations[1], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_LES:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR ||
                        operation_exec(op->data.operations[1], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_DIC:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_FIND:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR ||
                        operation_exec(op->data.operations[1], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_SPLIT:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR ||
                        operation_exec(op->data.operations[1], env) == RET_ERROR)
                            ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_ABS:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_SCOPE:
                    environment_add_scope(env);
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    environment_remove_scope(env);
                    break;
                case OPERATION_TYPE_IF: {
                    object_t** cond = operation_result(op->data.operations[0], env);

                    if(cond == NULL) {
                        error("Runtime error: If NULL error.");
                        ret = RET_ERROR;
                    } else if(cond == RET_ERROR) {
                        ret = RET_ERROR;
                    } else if(cond[1] != NULL) {
                        error("Runtime error: If non-scalar condition error.");
                        ret = RET_ERROR;
                    } else {
                        if(is_true(cond[0]))
                            if(operation_exec(op->data.operations[1], env) == RET_ERROR)
                                ret = RET_ERROR;
                    }

                    if(cond != RET_ERROR && cond != NULL) {
                        for(int i = 0; cond[i] != NULL; i++)
                            object_dereference(cond[i]);
                        _free(cond);
                    }
                } break;
                case OPERATION_TYPE_IFELSE: {
                    object_t** cond = operation_result(op->data.operations[0], env);

                    if(cond == NULL) {
                        error("Runtime error: If-else NULL error.");
                        ret = RET_ERROR;
                    } else if(cond == RET_ERROR) {
                        ret = RET_ERROR;
                    } else if(cond[1] != NULL) {
                        error("Runtime error: If-else non-scalar condition error.");
                        ret = RET_ERROR;
                    } else {
                        if(is_true(cond[0])) {
                            if(operation_exec(op->data.operations[1], env) == RET_ERROR)
                                ret = RET_ERROR;
                        } else {
                            if(operation_exec(op->data.operations[2], env) == RET_ERROR)
                                ret = RET_ERROR;
                        }
                    }

                    if(cond != RET_ERROR && cond != NULL) {
                        for(int i = 0; cond[i] != NULL; i++)
                            object_dereference(cond[i]);
                        _free(cond);
                    }
                } break;
                case OPERATION_TYPE_WHILE: {
                    object_t** cond = operation_result(op->data.operations[0], env);

                    if(cond == NULL) {
                        error("Runtime error: While NULL error.");
                        ret = RET_ERROR;
                    } else if(cond == RET_ERROR) {
                        ret = RET_ERROR;
                    } else if(cond[1] != NULL) {
                        error("Runtime error: While non-scalar condition error.");
                        ret = RET_ERROR;
                    } else
                        while(ret != RET_ERROR && is_true(cond[0]))
                        {
                            if(operation_exec(op->data.operations[1], env) == RET_ERROR)
                                ret = RET_ERROR;
                            else {
                                object_dereference(cond[0]);
                                _free(cond);
                                cond = operation_result(op->data.operations[0], env);
                                if(cond == NULL) {
                                    error("Runtime error: While NULL error.");
                                    ret = RET_ERROR;
                                } else if(cond == RET_ERROR) {
                                    ret = RET_ERROR;
                                } else if(cond[1] != NULL) {
                                    error("Runtime error: While non-scalar condition error.");
                                    ret = RET_ERROR;
                                }
                            }
                        }

                    if(cond != RET_ERROR && cond != NULL) {
                        for(int i = 0; cond[i] != NULL; i++)
                            object_dereference(cond[i]);
                        _free(cond);
                    }
                } break;
                case OPERATION_TYPE_IN_STRUCT: {
                    object_t** stc = operation_result(op->data.operations[0], env);

                    if(stc == NULL) {
                        error("Runtime error: Struct NULL error.");
                        ret = RET_ERROR;
                    } else if(stc == RET_ERROR)
                        ret = RET_ERROR;
                    else
                        for(int i = 0; ret != RET_ERROR && stc[i] != NULL; i++)
                            if(stc[i]->type != OBJECT_TYPE_STRUCT) {
                                error("Runtime error: Struct type error.");
                                ret = RET_ERROR;
                            } else
                                if(struct_exec(stc[i]->data.stc, op->data.operations[1]) == RET_ERROR)
                                    ret = RET_ERROR;

                    if(stc != RET_ERROR && stc != NULL) {
                        for(int i = 0; stc[i] != NULL; i++)
                            object_dereference(stc[i]);
                        _free(stc);
                    }
                } break;
                case OPERATION_TYPE_LOCAL: {
                    size_t prev_limit = env->local_mode_limit;
                    environment_set_local_mode(env, env->count-1);
                    ret = operation_exec(op->data.operations[0], env);
                    environment_set_local_mode(env, prev_limit);
                } break;
                case OPERATION_TYPE_GLOBAL: {
                    size_t prev_limit = env->local_mode_limit;
                    environment_set_local_mode(env, 0);
                    ret = operation_exec(op->data.operations[0], env);
                    environment_set_local_mode(env, prev_limit);
                } break;
                case OPERATION_TYPE_COPY:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_FOR: {
                    operation_exec(op->data.operations[0], env);
                    object_t** cond = operation_result(op->data.operations[1], env);

                    if(cond == NULL) {
                        error("Runtime error: while NULL error.");
                        ret = RET_ERROR;
                    } else if(cond == RET_ERROR) {
                        ret = RET_ERROR;
                    } else if(cond[1] != NULL) {
                        error("Runtime error: while non-scalar condition error.");
                        ret = RET_ERROR;
                    } else
                        while(ret != RET_ERROR && is_true(cond[0]))
                        {
                            if(operation_exec(op->data.operations[3], env) == RET_ERROR)
                                ret = RET_ERROR;
                            else if(operation_exec(op->data.operations[2], env) == RET_ERROR)
                                ret = RET_ERROR;
                            else {
                                object_dereference(cond[1]);
                                _free(cond);
                                cond = operation_result(op->data.operations[1], env);
                                if(cond == NULL) {
                                    error("Runtime error: while NULL error.");
                                    ret = RET_ERROR;
                                } else if(cond == RET_ERROR) {
                                    ret = RET_ERROR;
                                } else if(cond[1] != NULL) {
                                    error("Runtime error: while non-scalar condition error.");
                                    ret = RET_ERROR;
                                }
                            }
                        }

                    if(cond != RET_ERROR && cond != NULL) {
                        object_dereference(cond[0]);
                        _free(cond);
                    }
                } break;
                case OPERATION_TYPE_LIST_OPEN:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_FOR_IN: {
                    object_t*** vals_loc = operation_var(op->data.operations[0], env);
                    object_t** vals_in = operation_result(op->data.operations[1], env);

                    if(vals_loc == NULL || vals_in == NULL) {
                        error("Runtime error: For-in NULL error.");
                        ret = RET_ERROR;
                    } else if(vals_loc == RET_ERROR || vals_in == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        int pos_in = 0;
                        while(ret != RET_ERROR && vals_in[pos_in] != NULL) {
                            // Assign values
                            for(int i = 0; vals_loc[i] != NULL && vals_in[pos_in] != NULL; i++) {
                                if(vals_loc[i] == OBJECT_LIST_OPENED) {
                                    object_t** obj = vals_loc[i+1];
                                    object_dereference(*obj);

                                    size_t length_left = 0;
                                    while(vals_in[pos_in + length_left] != NULL) length_left++;

                                    list_t* list = list_create_null(length_left);
                                    for(int j = 0; j < length_left; j++) {
                                        list->data[j] = vals_in[i+j];
                                        object_reference(list->data[j]);
                                    }
                                    *obj = object_create_list(list);
                                    object_reference(*obj);
                                    pos_in += length_left;
                                } else {
                                    object_dereference(*(vals_loc[i]));
                                    *(vals_loc[i]) = vals_in[pos_in];
                                    object_reference(*(vals_loc[i]));
                                    pos_in++;
                                }
                            }

                            if(operation_exec(op->data.operations[2], env) == RET_ERROR)
                                ret = RET_ERROR;
                        }
                    }
                    if(vals_loc != RET_ERROR && vals_loc != NULL) {
                        _free(vals_loc);
                    }
                    if(vals_in != RET_ERROR && vals_in != NULL) {
                        for(int i = 0; vals_in[i] != NULL; i++)
                            object_dereference(vals_in[i]);
                        _free(vals_in);
                    }
                } break;
                case OPERATION_TYPE_IMPORT: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Import NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        for (int i = 0; ret != RET_ERROR && vals[i] != NULL; i++) {
                            if(vals[i]->type != OBJECT_TYPE_STRING) {
                                error("Runtime error: Import type error.");
                                ret = RET_ERROR;
                            } else {
                                FILE* file = fopen(string_get_cstr(vals[i]->data.string), "r");
                                if(file == NULL) {
                                    error("Runtime error: Import file error.");
                                    ret = RET_ERROR;
                                } else {
                                    old_error_handler = get_error_handler();
                                    set_error_handler(import_error_handler);

                                    fseek(file, 0, SEEK_END);
                                    size_t file_size = ftell(file);
                                    fseek(file, 0, SEEK_SET);

                                    char* buffer = (char*)_alloc(sizeof(char)*(file_size+1));
                                    buffer[file_size] = '\0';
                                    fread(buffer, 1, file_size, file);

                                    program_t* program = tokenize_and_parse_program(buffer);
                                    program_exec(program, env);
                                    program_free(program);

                                    _free(buffer);
                                    fclose(file);

                                    set_error_handler(old_error_handler);

                                    if(import_error_flag) {
                                        ret = RET_ERROR;
                                    }
                                }
                            }
                        }
                    }
                } break;
                case OPERATION_TYPE_FOPEN:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_FCLOSE: {
                    object_t** data = operation_result(op->data.operations[0], env);

                    if(data == NULL) {
                        error("Runtime error: fclose NULL error.");
                        ret = RET_ERROR;
                    } else if(data == RET_ERROR)
                        ret = RET_ERROR;
                    else if(data[1] != NULL) {
                        error("Runtime error: fclose Too many arguments error.");
                        ret = RET_ERROR;
                    } else if(data[0]->type != OBJECT_TYPE_NUMBER) {
                        error("Runtime error: fclose Type error.");
                        ret = RET_ERROR;
                    } else if(data[0]->data.number != (int)(data[0]->data.number)) {
                        error("Runtime error: fclose Integer error.");
                        ret = RET_ERROR;
                    } else {
                        int file = (int)(data[0]->data.number);
                        close(file);
                    }
                    if(data != RET_ERROR && data != NULL) {
                        for(int i = 0; data[i] != NULL; i++)
                            object_dereference(data[i]);
                        _free(data);
                    }
                } break;
                case OPERATION_TYPE_FREAD:
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR)
                        ret = RET_ERROR;
                    break;
                case OPERATION_TYPE_FWRITE: {
                    object_t** data = operation_result(op->data.operations[0], env);

                    if(data == NULL) {
                        error("Runtime error: fwrite NULL error.");
                        ret = RET_ERROR;
                    } else if(data == RET_ERROR)
                        ret = RET_ERROR;
                    else if(data[0]->type != OBJECT_TYPE_NUMBER) {
                        error("Runtime error: fwrite Type error.");
                        ret = RET_ERROR;
                    } else if(data[0]->data.number != (int)(data[0]->data.number)) {
                        error("Runtime error: fwrite Integer error.");
                        ret = RET_ERROR;
                    } else {
                        int file = (int)(data[0]->data.number);
                        for(int i = 1; data[i] != NULL; i++) {
                            string_t* str = object_to_string(data[i]);
                            write(file, str->data, str->length);
                            string_free(str);
                        }
                    }
                    if(data != RET_ERROR && data != NULL) {
                        for(int i = 0; data[i] != NULL; i++)
                            object_dereference(data[i]);
                        _free(data);
                    }
                } break;
            }
        }

    return ret;
}

object_t** operation_result(operation_t* op, environment_t* env) {
    object_t** ret = NULL;

    if(check_for_stackoverflow()) {
        error("Runtime error: Stack overflow.");
        ret = RET_ERROR;
    } else
        if(op != NULL) {
            switch(op->type) {
                case OPERATION_TYPE_NOOP:
                case OPERATION_TYPE_NOOP_BRAC:
                case OPERATION_TYPE_NOOP_EMP_REC:
                case OPERATION_TYPE_NOOP_O_LIST_DEADEND:
                case OPERATION_TYPE_NOOP_PROC_DEADEND:
                case OPERATION_TYPE_NOOP_PLUS:
                case OPERATION_TYPE_NOOP_EMP_CUR: break;
                case OPERATION_TYPE_NONE:
                    ret = (object_t**)_alloc(sizeof(object_t*)*2);
                    ret[0] = object_create_none();
                    object_reference(ret[0]);
                    ret[1] = NULL;
                    break;
                case OPERATION_TYPE_NUM:
                    ret = (object_t**)_alloc(sizeof(object_t*)*2);
                    ret[0] = object_create_number(op->data.num);
                    object_reference(ret[0]);
                    ret[1] = NULL;
                    break;
                case OPERATION_TYPE_STR:
                    ret = (object_t**)_alloc(sizeof(object_t*)*2);
                    ret[0] = object_create_string(string_copy(op->data.str));
                    object_reference(ret[0]);
                    ret[1] = NULL;
                    break;
                case OPERATION_TYPE_VAR:
                    ret = (object_t**)_alloc(sizeof(object_t*)*2);
                    environment_make(env, op->data.str);
                    ret[0] = environment_get(env, op->data.str);
                    ret[1] = NULL;
                    if(ret[0]->type == OBJECT_TYPE_MACRO) {
                        object_t** tmp = macro_result(ret[0]->data.mac, env);
                        if(tmp == RET_ERROR) {
                            _free(ret);
                            ret = RET_ERROR;
                        } else {
                            _free(ret);
                            ret = tmp;
                        }
                    } else
                        object_reference(ret[0]);
                    break;
                case OPERATION_TYPE_BOOL:
                    ret = (object_t**)_alloc(sizeof(object_t*)*2);
                    ret[0] = object_create_boolean(op->data.boolean);
                    object_reference(ret[0]);
                    ret[1] = NULL;
                    break;
                case OPERATION_TYPE_PAIR: {
                    object_t** keys = operation_result(op->data.operations[0], env);
                    object_t** vals = operation_result(op->data.operations[1], env);

                    if(keys == NULL || vals == NULL) {
                        error("Runtime error: Pair NULL error.");
                        ret = RET_ERROR;
                    } else if(keys == RET_ERROR || vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t keys_len = 0;
                        while(keys[keys_len] != NULL) keys_len++;
                        size_t vals_len = 0;
                        while(vals[vals_len] != NULL) vals_len++;

                        if(keys_len > 1 && vals_len > 1) {
                            if(keys_len != vals_len) {
                                error("Runtime error: Pair symmetry error");
                                ret = RET_ERROR;
                            } else {
                                ret = (object_t**)_alloc(sizeof(object_t*)*(vals_len + 1));

                                for(int i = 0; i < vals_len; i++) {
                                    ret[i] = object_create_pair(pair_create(keys[i], vals[i]));
                                    object_reference(ret[i]);
                                }
                                ret[vals_len] = NULL;
                            }
                        } else if(keys_len < vals_len) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(vals_len + 1));

                            for(int i = 0; i < vals_len; i++) {
                                ret[i] = object_create_pair(pair_create(keys[0], vals[i]));
                                object_reference(ret[i]);
                            }
                            ret[vals_len] = NULL;
                        } else {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(keys_len + 1));

                            for(int i = 0; i < keys_len; i++) {
                                ret[i] = object_create_pair(pair_create(keys[i], vals[0]));
                                object_reference(ret[i]);
                            }
                            ret[keys_len] = NULL;
                        }
                    }

                    if(keys != RET_ERROR && keys != NULL) {
                        for(int i = 0; keys[i] != NULL; i++)
                            object_dereference(keys[i]);
                        _free(keys);
                    }
                    if(vals != RET_ERROR && vals != NULL) {
                        for(int i = 0; vals[i] != NULL; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_FUNCTION:
                    ret = (object_t**)_alloc(sizeof(object_t*)*2);
                    ret[0] = object_create_function(function_create(op->data.operations[0], op->data.operations[1]));
                    object_reference(ret[0]);
                    ret[1] = NULL;
                    break;
                case OPERATION_TYPE_MACRO:
                    ret = (object_t**)_alloc(sizeof(object_t*)*2);
                    ret[0] = object_create_macro(macro_create(op->data.operations[0]));
                    object_reference(ret[0]);
                    ret[1] = NULL;
                    break;
                case OPERATION_TYPE_ASSIGN: {
                    object_t**    assign_val = operation_result(op->data.operations[1], env);
                    object_t*** assign_loc = operation_var(op->data.operations[0], env);

                    if(assign_loc == NULL || assign_val == NULL) {
                        error("Runtime error: Assignment NULL error.");
                        ret = RET_ERROR;
                    } else if(assign_loc == RET_ERROR || assign_val == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        for(int i = 0; assign_loc[i] != NULL && assign_val[i] != NULL; i++) {
                            if(assign_loc[i] == OBJECT_LIST_OPENED) {
                                object_t** obj = assign_loc[i+1];
                                object_dereference(*obj);

                                size_t length_left = 0;
                                while(assign_val[i + length_left] != NULL) length_left++;

                                list_t* list = list_create_null(length_left);
                                for(int j = 0; j < length_left; j++) {
                                    list->data[j] = assign_val[i+j];
                                    object_reference(list->data[j]);
                                }
                                *obj = object_create_list(list);
                                object_reference(*obj);
                                break;
                            } else {
                                object_dereference(*(assign_loc[i]));
                                *(assign_loc[i]) = assign_val[i];
                                object_reference(*(assign_loc[i]));
                            }
                        }
                    }

                    if(ret != RET_ERROR)
                        ret = assign_val;
                    else if(assign_val != RET_ERROR && assign_val != NULL) {
                        for(int i = 0; assign_val[i] != NULL; i++)
                            object_dereference(assign_val[i]);
                        _free(assign_val);
                    }
                    if(assign_loc != RET_ERROR && assign_loc != NULL)
                        _free(assign_loc);
                    } break;
                case OPERATION_TYPE_PROC:
                case OPERATION_TYPE_PROC_IMP: {
                    int i;
                    for(i = 0; ret != RET_ERROR && op->data.operations[i+1] != NULL; i++)
                        if(operation_exec(op->data.operations[i], env) == RET_ERROR)
                            ret = RET_ERROR;
                    if(ret != RET_ERROR)
                        ret = operation_result(op->data.operations[i], env);
                } break;
                case OPERATION_TYPE_STRUCT: {
                    ret = (object_t**)_alloc(sizeof(object_t*)*2);
                    ret[0] = object_create_struct(struct_create());
                    object_reference(ret[0]);
                    ret[1] = NULL;
                    string_t* name_self = string_create("self");
                    environment_write(ret[0]->data.stc, name_self, ret[0]);
                    object_dereference(ret[0]);    // Since the object contains itfels derefrencing helps to prevent loops (Better garbage collector is required)
                    string_free(name_self);
                    if(struct_exec(ret[0]->data.stc, op->data.operations[0]) == RET_ERROR) {
                        object_dereference(ret[0]);
                        _free(ret);
                        ret = RET_ERROR;
                    }
                } break;
                case OPERATION_TYPE_O_LIST: {
                    size_t num_op = 0;
                    while(op->data.operations[num_op] != NULL) num_op++;

                    object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

                    size_t num_ret = 0;
                    for(int i = 0; ret != RET_ERROR && i < num_op; i++) {
                        vals[i] = operation_result(op->data.operations[i], env);
                        if(vals[i] == NULL) {
                            error("Runtime error: Open list NULL error.");
                            ret = RET_ERROR;
                            for(++i;i < num_op; i++)
                                vals[i] = NULL;
                        } else if (vals[i] == RET_ERROR) {
                            ret = RET_ERROR;
                            for(++i;i < num_op; i++)
                                vals[i] = NULL;
                        } else
                            for(int j = 0; vals[i][j] != NULL; j++)
                                num_ret++;
                    }
                    if(ret != RET_ERROR) {
                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
                        num_ret = 0;
                        for(int i = 0; i < num_op; i++) {
                            for(int j = 0; vals[i][j] != NULL; j++)
                                ret[num_ret++] = vals[i][j];
                        }
                        ret[num_ret] = NULL;
                    }

                    for(int i = 0; i < num_op; i++)
                        if(ret != RET_ERROR) {
                            _free(vals[i]);
                        } else {
                            if(vals[i] != RET_ERROR && vals[i] != NULL) {
                                for(int j = 0; vals[i][j] != NULL; j++)
                                    object_dereference(vals[i][j]);
                                _free(vals[i]);
                            }
                        }
                    _free(vals);
                } break;
                case OPERATION_TYPE_LIST: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        list_t* list = list_create_empty();;

                        if(vals != NULL) {
                            size_t num_vals = 0;
                            while(vals[num_vals] != NULL) num_vals++;
                            list->data = vals;
                            list->size = num_vals;
                        }

                        ret = (object_t**)_alloc(sizeof(object_t*)*2);
                        ret[0] = object_create_list(list);
                        object_reference(ret[0]);
                        ret[1] = NULL;
                    }
                } break;
                case OPERATION_TYPE_INDEX: {
                    object_t** data = operation_result(op->data.operations[0], env);
                    object_t** index = operation_result(op->data.operations[1], env);

                    if(data == NULL || index == NULL) {
                        error("Runtime error: Indexing NULL error.");
                        ret = RET_ERROR;
                    } else if(data == RET_ERROR || index == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_data = 0;
                        while(data[num_data] != NULL) num_data++;
                        size_t num_index = 0;
                        while(index[num_index] != NULL) num_index++;
                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_data*num_index+1));

                        for(int i = 0; ret != RET_ERROR && i < num_data; i++)
                            for(int j = 0; ret != RET_ERROR && j < num_index; j++) {
                                if(data[i]->type == OBJECT_TYPE_LIST) {

                                    if(index[j]->type == OBJECT_TYPE_NUMBER) {

                                        pos_t ind = (int)round(index[j]->data.number);
                                        if(ind < 0)
                                            ind += data[i]->data.list->size;
                                        object_t* obj = list_get(data[i]->data.list, ind);
                                        if(obj == NULL)
                                            ret[i*num_index+j] = object_create_none();
                                        else
                                            ret[i*num_index+j] = obj;
                                        object_reference(ret[i*num_index+j]);

                                    } else if (index[j]->type == OBJECT_TYPE_PAIR && index[j]->data.pair->key->type == OBJECT_TYPE_NUMBER && index[j]->data.pair->value->type == OBJECT_TYPE_NUMBER) {

                                        pos_t index_start = (int)round(index[j]->data.pair->key->data.number);
                                        pos_t index_end = (int)round(index[j]->data.pair->value->data.number);
                                        if(index_start < 0)
                                            index_start += data[i]->data.list->size;
                                        if(index_end < 0)
                                            index_end += data[i]->data.list->size;

                                        list_t* list = list_range(data[i]->data.list, index_start, (index_end - index_start) > 0 ? (index_end - index_start + 1) : (index_end - index_start - 1));
                                        if(list == NULL)
                                            ret[i*num_index+j] = object_create_list(list_create_empty());
                                        else
                                            ret[i*num_index+j] = object_create_list(list);
                                        object_reference(ret[i*num_index+j]);

                                    } else {
                                        error("Runtime error: Indexing type error.");
                                        for(int k = 0; k < i*num_index+j; k++)
                                            object_dereference(ret[k]);
                                        _free(ret);
                                        ret = RET_ERROR;
                                    }

                                } else if (data[i]->type == OBJECT_TYPE_DICTIONARY) {

                                    ret[i*num_index+j] = dictionary_get(data[i]->data.dic, index[j]);
                                    if(ret[i*num_index+j] == NULL)
                                        ret[i*num_index+j] = object_create_none();
                                    object_reference(ret[i*num_index+j]);

                                } else if (data[i]->type == OBJECT_TYPE_STRING) {

                                    if(index[j]->type == OBJECT_TYPE_NUMBER) {

                                        pos_t ind = (int)round(index[j]->data.number);
                                        if(index < 0)
                                            ind += data[i]->data.string->length;

                                        string_t* str = string_substr(data[i]->data.string, ind, 1);
                                        if(str == NULL)
                                            ret[i*num_index+j] = object_create_none();
                                        else
                                            ret[i*num_index+j] = object_create_string(str);
                                        object_reference(ret[i*num_index+j]);

                                    } else if (index[j]->type == OBJECT_TYPE_PAIR && index[j]->data.pair->key->type == OBJECT_TYPE_NUMBER && index[j]->data.pair->value->type == OBJECT_TYPE_NUMBER) {

                                        pos_t index_start = (int)round(index[j]->data.pair->key->data.number);
                                        pos_t index_end = (int)round(index[j]->data.pair->value->data.number);
                                        if(index_start < 0)
                                            index_start += data[i]->data.string->length;
                                        if(index_end < 0)
                                            index_end += data[i]->data.string->length;

                                        string_t* str = string_substr(data[i]->data.string, index_start, (index_end - index_start) > 0 ? (index_end - index_start + 1) : (index_end - index_start - 1));
                                        if(str == NULL)
                                            ret[i*num_index+j] = object_create_string(string_create(""));
                                        else
                                            ret[i*num_index+j] = object_create_string(str);
                                        object_reference(ret[i*num_index+j]);

                                    } else {
                                        error("Runtime error: Indexing type error.");
                                        for(int k = 0; k < i*num_index+j; k++)
                                            object_dereference(ret[k]);
                                        _free(ret);
                                        ret = RET_ERROR;
                                    }

                                } else {
                                    error("Runtime error: Indexing type error.");
                                    for(int k = 0; k < i*num_index+j; k++)
                                        object_dereference(ret[k]);
                                    _free(ret);
                                    ret = RET_ERROR;
                                }
                            }
                        if(ret != RET_ERROR)
                            ret[num_data*num_index] = NULL;
                    }


                    if(data != RET_ERROR && data != NULL) {
                        for(int i = 0; data[i] != NULL; i++)
                            object_dereference(data[i]);
                        _free(data);
                    }
                    if(index != RET_ERROR && index != NULL) {
                        for(int i = 0; index[i] != NULL; i++)
                            object_dereference(index[i]);
                        _free(index);
                    }

                } break;
                case OPERATION_TYPE_EXEC: {
                    object_t** function = operation_result(op->data.operations[0], env);
                    object_t** parameter = operation_result(op->data.operations[1], env);

                    if(function == NULL) {
                        error("Runtime error: Function NULL error.");
                        ret = RET_ERROR;
                    } else if(function == RET_ERROR || parameter == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_func = 0;
                        while(function[num_func] != NULL) num_func++;

                        object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_func);

                        size_t num_ret = 0;
                        for(int i = 0; ret != RET_ERROR && i < num_func; i++) {
                            if(function[i]->type != OBJECT_TYPE_FUNCTION) {
                                error("Runtime error: Function type error.");
                                ret = RET_ERROR;
                                for(;i < num_func; i++)
                                    vals[i] = NULL;
                            } else {
                                vals[i] = function_result(function[i]->data.func, parameter, env);
                                if(vals[i] == NULL) {
                                    error("Runtime error: Function NULL error");
                                    ret = RET_ERROR;
                                    for(++i;i < num_func; i++)
                                        vals[i] = NULL;
                                } else if (vals[i] == RET_ERROR) {
                                    ret = RET_ERROR;
                                    for(++i;i < num_func; i++)
                                        vals[i] = NULL;
                                } else
                                    for(int j = 0; vals[i][j] != NULL; j++)
                                        num_ret++;
                            }
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
                            num_ret = 0;
                            for(int i = 0; i < num_func; i++)
                                for(int j = 0; vals[i][j] != NULL; j++) {
                                    ret[num_ret] = vals[i][j];
                                    num_ret++;
                                }
                            ret[num_ret] = NULL;
                        }

                        for(int i = 0; i < num_func; i++)
                            if(ret != RET_ERROR) {
                                _free(vals[i]);
                            } else {
                                if(vals[i] != RET_ERROR && vals[i] != NULL) {
                                    for(int j = 0; vals[i][j] != NULL; j++)
                                        object_dereference(vals[i][j]);
                                    _free(vals[i]);
                                }
                            }
                        _free(vals);
                    }

                    if(function != RET_ERROR && function != NULL) {
                        for(int i = 0; function[i] != NULL; i++)
                            object_dereference(function[i]);
                        _free(function);
                    }
                    if(parameter != RET_ERROR && parameter != NULL) {
                        for(int i = 0; parameter[i] != NULL; i++)
                            object_dereference(parameter[i]);
                        _free(parameter);
                    }
                } break;
                case OPERATION_TYPE_TO_NUM: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: To_num NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(vals[num_vals] != NULL) num_vals++;

                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                        for(int i = 0; ret != RET_ERROR && i < num_vals; i++) {
                            switch (vals[i]->type) {
                                case OBJECT_TYPE_NONE: ret[i] = object_create_number(0); break;
                                case OBJECT_TYPE_NUMBER: ret[i] = vals[i]; break;
                                case OBJECT_TYPE_BOOL: ret[i] = object_create_number(vals[i]->data.boolean ? 1 : 0); break;
                                case OBJECT_TYPE_STRING: ret[i] = object_create_number(atof(string_get_cstr(vals[i]->data.string))); break;
                                case OBJECT_TYPE_PAIR:
                                case OBJECT_TYPE_LIST:
                                case OBJECT_TYPE_DICTIONARY:
                                case OBJECT_TYPE_FUNCTION:
                                case OBJECT_TYPE_MACRO:
                                case OBJECT_TYPE_STRUCT:
                                    error("Runtime error: To_num type error.");
                                    for(int j = 0; j < i; j++)
                                        object_dereference(ret[j]);
                                    _free(ret);
                                    ret = RET_ERROR;
                                    break;
                            }
                            if(ret != RET_ERROR)
                                object_reference(ret[i]);
                        }
                        if(ret != RET_ERROR)
                            ret[num_vals] = NULL;

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_TO_BOOL: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: To_bool NULL error.");
                        ret = RET_ERROR;
                    } else if (vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(vals[num_vals] != NULL) num_vals++;

                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                        for(int i = 0; i < num_vals; i++) {
                            ret[i] = object_create_boolean(is_true(vals[i]));
                            object_reference(ret[i]);
                        }

                        ret[num_vals] = NULL;
                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_TO_ASCII: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: To_ascii NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(vals[num_vals] != NULL) num_vals++;

                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                        for(int i = 0; ret != RET_ERROR && i < num_vals; i++) {
                            switch (vals[i]->type) {
                                case OBJECT_TYPE_NUMBER: {
                                    char tmp[2];
                                    if(vals[i]->data.number != round(vals[i]->data.number)) {
                                        error("Runtime error: To_ascii non-integer error.");
                                        for(int j = 0; j < i; j++)
                                            object_dereference(ret[i]);
                                        _free(ret);
                                        ret = RET_ERROR;
                                    } else {
                                        tmp[0] = (char)(int)round(vals[i]->data.number);
                                        tmp[1] = '\0';
                                        ret[i] = object_create_string(string_create(tmp));
                                    }
                                } break;
                                case OBJECT_TYPE_STRING: ret[i] = vals[i]; break;
                                case OBJECT_TYPE_LIST: {
                                    char tmp[vals[i]->data.list->size+1];
                                    tmp[vals[i]->data.list->size] = '\0';

                                    for(int j = 0; ret != RET_ERROR && j < vals[i]->data.list->size; j++) {
                                        if(vals[i]->data.list->data[j]->type != OBJECT_TYPE_NUMBER) {
                                            error("Runtime error: To_ascii type error.");
                                            for(int j = 0; j < i; j++)
                                                object_dereference(ret[i]);
                                            _free(ret);
                                            ret = RET_ERROR;
                                        } else if(vals[i]->data.list->data[j]->data.number != round(vals[i]->data.list->data[j]->data.number)) {
                                            error("Runtime error: To_ascii non-integer error.");
                                            for(int j = 0; j < i; j++)
                                                object_dereference(ret[i]);
                                            _free(ret);
                                            ret = RET_ERROR;
                                        } else
                                            tmp[j] = (char)(int)round(vals[i]->data.list->data[j]->data.number);
                                    }

                                    if(ret != RET_ERROR)
                                        ret[i] = object_create_string(string_create(tmp));
                                } break;
                                case OBJECT_TYPE_BOOL:
                                case OBJECT_TYPE_NONE:
                                case OBJECT_TYPE_PAIR:
                                case OBJECT_TYPE_DICTIONARY:
                                case OBJECT_TYPE_FUNCTION:
                                case OBJECT_TYPE_MACRO:
                                case OBJECT_TYPE_STRUCT:
                                    error("Runtime error: To_ascii type error.");
                                    for(int j = 0; j < i; j++)
                                        object_dereference(ret[i]);
                                    _free(ret);
                                    ret = RET_ERROR;
                                    break;
                            }
                            if(ret != RET_ERROR)
                                object_reference(ret[i]);
                        }
                        if(ret != RET_ERROR)
                            ret[num_vals] = NULL;

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_TO_STR: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: To_bool NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(vals[num_vals] != NULL) num_vals++;

                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                        for(int i = 0; i < num_vals; i++) {
                            if(vals[i]->type == OBJECT_TYPE_STRING)
                                ret[i] = vals[i];
                            else
                                ret[i] = object_create_string(object_to_string(vals[i]));
                            object_reference(ret[i]);
                        }
                        ret[num_vals] = NULL;

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_READ: {
                    char temp_str[TMP_STR_MAX];
                    fgets(temp_str, TMP_STR_MAX, stdin);
                    ret = (object_t**)_alloc(sizeof(object_t*)*2);
                    ret[0] = object_create_string(string_create(temp_str));
                    object_reference(ret[0]);
                    ret[1] = NULL;
                } break;
                case OPERATION_TYPE_WRITE: {
                    object_t** data = operation_result(op->data.operations[0], env);

                    if(data == NULL) {
                        error("Runtime error: Write NULL error.");
                        ret = RET_ERROR;
                    } else if(data == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        for(int i = 0; data[i] != NULL; i++)
                            print_object(data[i]);

                        for(int i = 0; data[i] != NULL; i++)
                            object_dereference(data[i]);
                        _free(data);
                    }
                } break;
                case OPERATION_TYPE_ADD: {
                    size_t num_op = 0;
                    while(op->data.operations[num_op] != NULL) num_op++;

                    object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

                    size_t num_ret = 1;
                    for(int i = 0; ret != RET_ERROR && i < num_op; i++) {
                        vals[i] = operation_result(op->data.operations[i], env);
                        if(vals[i] == NULL) {
                            error("Runtime error: Addition NULL error.");
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else if(vals[i] == RET_ERROR) {
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else {
                            size_t tmp_size = 0;
                            while(vals[i][tmp_size] != NULL) {
                                tmp_size++;
                            }
                            if(num_ret == 1 && tmp_size != 1)
                                num_ret = tmp_size;
                            else if(num_ret != tmp_size && tmp_size != 1) {
                                error("Runtime error: Addition symmetry error.");
                                ret = RET_ERROR;
                                for(++i; i < num_op; i++)
                                    vals[i] = NULL;
                            }
                        }
                    }
                    if(ret != RET_ERROR) {
                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
                        for(int i = 0; ret != RET_ERROR && i < num_ret; i++) {
                            if(vals[0][1] == NULL)
                                ret[i] = vals[0][0];
                            else
                                ret[i] = vals[0][i];
                            object_reference(ret[i]);
                            object_t* tmp;

                            for(int j = 1; ret != RET_ERROR && j < num_op; j++) {
                                if(vals[j][1] == NULL)
                                    tmp = object_add(ret[i], vals[j][0]);
                                else
                                    tmp = object_add(ret[i], vals[j][i]);

                                if(tmp == RET_ERROR) {
                                    for(int k = 0; k <= i; k++)
                                        object_dereference(ret[i]);
                                    _free(ret);
                                    ret = RET_ERROR;
                                } else {
                                    object_dereference(ret[i]);
                                    ret[i] = tmp;
                                    object_reference(tmp);
                                }
                            }
                        }
                        if(ret != RET_ERROR)
                            ret[num_ret] = NULL;
                    }

                    for(int i = 0; i < num_op; i++) {
                        if(vals[i] != RET_ERROR && vals[i] != NULL) {
                            for(int j = 0; vals[i][j] != NULL; j++)
                                object_dereference(vals[i][j]);
                            _free(vals[i]);
                        }
                    }
                    _free(vals);
                } break;
                case OPERATION_TYPE_SUB: {
                    size_t num_op = 0;
                    while(op->data.operations[num_op] != NULL) num_op++;

                    object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

                    size_t num_ret = 1;
                    for(int i = 0; ret != RET_ERROR && i < num_op; i++) {
                        vals[i] = operation_result(op->data.operations[i], env);
                        if(vals[i] == NULL) {
                            error("Runtime error: Subtraction NULL error.");
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else if(vals[i] == RET_ERROR) {
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else {
                            size_t tmp_size = 0;
                            while(ret != RET_ERROR && vals[i][tmp_size] != NULL) {
                                if(vals[i][tmp_size]->type != OBJECT_TYPE_NUMBER) {
                                    error("Runtime error: Subtraction type error.");
                                    ret = RET_ERROR;
                                    for(++i; i < num_op; i++)
                                        vals[i] = NULL;
                                } else
                                    tmp_size++;
                            }
                            if(ret != RET_ERROR) {
                                if(num_ret == 1 && tmp_size != 1)
                                    num_ret = tmp_size;
                                else if(num_ret != tmp_size && tmp_size != 1) {
                                    error("Runtime error: Subtraction symmetry error.");
                                    ret = RET_ERROR;
                                    for(++i; i < num_op; i++)
                                        vals[i] = NULL;
                                }
                            }
                        }
                    }
                    if(ret != RET_ERROR) {
                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
                        for(int i = 0; i < num_ret; i++) {
                            number_t ret_part;
                            if(vals[0][1] == NULL)
                                ret_part = vals[0][0]->data.number;
                            else
                                ret_part = vals[0][i]->data.number;

                            for(int j = 1; j < num_op; j++) {
                                if(vals[j][1] == NULL)
                                    ret_part -= vals[j][0]->data.number;
                                else
                                    ret_part -= vals[j][i]->data.number;
                            }

                            ret[i] = object_create_number(ret_part);
                            object_reference(ret[i]);
                        }
                        ret[num_ret] = NULL;
                    }

                    for(int i = 0; i < num_op; i++) {
                        if(vals[i] != RET_ERROR && vals[i] != NULL) {
                            for(int j = 0; vals[i][j] != NULL; j++)
                                object_dereference(vals[i][j]);
                            _free(vals[i]);
                        }
                    }
                    _free(vals);
                } break;
                case OPERATION_TYPE_MUL: {
                    size_t num_op = 0;
                    while(op->data.operations[num_op] != NULL) num_op++;

                    object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

                    size_t num_ret = 1;
                    for(int i = 0; ret != RET_ERROR && i < num_op; i++) {
                        vals[i] = operation_result(op->data.operations[i], env);
                        if(vals[i] == NULL) {
                            error("Runtime error: Multiplication NULL error.");
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else if(vals[i] == RET_ERROR) {
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else {
                            size_t tmp_size = 0;
                            while(vals[i][tmp_size] != NULL) {
                                tmp_size++;
                            }
                            if(num_ret == 1 && tmp_size != 1)
                                num_ret = tmp_size;
                            else if(num_ret != tmp_size && tmp_size != 1) {
                                error("Runtime error: Multiplication symmetry error.");
                                ret = RET_ERROR;
                                for(++i; i < num_op; i++)
                                    vals[i] = NULL;
                            }
                        }
                    }
                    if(ret != RET_ERROR) {
                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
                        for(int i = 0; i < num_ret; i++) {
                            if(vals[0][1] == NULL)
                                ret[i] = vals[0][0];
                            else
                                ret[i] = vals[0][i];
                            object_reference(ret[i]);
                            object_t* tmp;

                            for(int j = 1; ret != RET_ERROR && j < num_op; j++) {
                                if(vals[j][1] == NULL)
                                    tmp = object_mul(ret[i], vals[j][0]);
                                else
                                    tmp = object_mul(ret[i], vals[j][i]);

                                if(tmp == RET_ERROR) {
                                    for(int k = 0; k <= i; k++)
                                        object_dereference(ret[i]);
                                    _free(ret);
                                    ret = RET_ERROR;
                                } else {
                                    object_dereference(ret[i]);
                                    ret[i] = tmp;
                                    object_reference(tmp);
                                }
                            }
                        }
                        if(ret != RET_ERROR)
                            ret[num_ret] = NULL;
                    }

                    for(int i = 0; i < num_op; i++) {
                        if(vals[i] != RET_ERROR && vals[i] != NULL) {
                            for(int j = 0; vals[i][j] != NULL; j++)
                                object_dereference(vals[i][j]);
                            _free(vals[i]);
                        }
                    }
                    _free(vals);
                } break;
                case OPERATION_TYPE_DIV: {
                    size_t num_op = 0;
                    while(op->data.operations[num_op] != NULL) num_op++;

                    object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

                    size_t num_ret = 1;
                    for(int i = 0; ret != RET_ERROR && i < num_op; i++) {
                        vals[i] = operation_result(op->data.operations[i], env);
                        if(vals[i] == NULL) {
                            error("Runtime error: Division NULL error.");
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else if(vals[i] == RET_ERROR) {
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else {
                            size_t tmp_size = 0;
                            while(ret != RET_ERROR && vals[i][tmp_size] != NULL) {
                                if(vals[i][tmp_size]->type != OBJECT_TYPE_NUMBER) {
                                    error("Runtime error: Division type error.");
                                    ret = RET_ERROR;
                                    for(++i; i < num_op; i++)
                                        vals[i] = NULL;
                                } else
                                    tmp_size++;
                            }
                            if(ret != RET_ERROR) {
                                if(num_ret == 1 && tmp_size != 1)
                                    num_ret = tmp_size;
                                else if(num_ret != tmp_size && tmp_size != 1) {
                                    error("Runtime error: Division symmetry error.");
                                    ret = RET_ERROR;
                                    for(++i; i < num_op; i++)
                                        vals[i] = NULL;
                                }
                            }
                        }
                    }
                    if(ret != RET_ERROR) {
                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
                        for(int i = 0; i < num_ret; i++) {
                            number_t ret_part;
                            if(vals[0][1] == NULL)
                                ret_part = vals[0][0]->data.number;
                            else
                                ret_part = vals[0][i]->data.number;

                            for(int j = 1; j < num_op; j++) {
                                if(vals[j][1] == NULL)
                                    ret_part /= vals[j][0]->data.number;
                                else
                                    ret_part /= vals[j][i]->data.number;
                            }

                            ret[i] = object_create_number(ret_part);
                            object_reference(ret[i]);
                        }
                        ret[num_ret] = NULL;
                    }

                    for(int i = 0; i < num_op; i++) {
                        if(vals[i] != RET_ERROR && vals[i] != NULL) {
                            for(int j = 0; vals[i][j] != NULL; j++)
                                object_dereference(vals[i][j]);
                            _free(vals[i]);
                        }
                    }
                    _free(vals);
                } break;
                case OPERATION_TYPE_MOD: {
                    size_t num_op = 0;
                    while(op->data.operations[num_op] != NULL) num_op++;

                    object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

                    size_t num_ret = 1;
                    for(int i = 0; ret != RET_ERROR && i < num_op; i++) {
                        vals[i] = operation_result(op->data.operations[i], env);
                        if(vals[i] == NULL) {
                            error("Runtime error: Modulo NULL error.");
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else if(vals[i] == RET_ERROR) {
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else {
                            size_t tmp_size = 0;
                            while(ret != RET_ERROR && vals[i][tmp_size] != NULL) {
                                if(vals[i][tmp_size]->type != OBJECT_TYPE_NUMBER) {
                                    error("Runtime error: Modulo type error.");
                                    ret = RET_ERROR;
                                    for(++i; i < num_op; i++)
                                        vals[i] = NULL;
                                } else if(vals[i][tmp_size]->data.number != round(vals[i][tmp_size]->data.number)) {
                                    error("Runtime error: Modulo non-integer Error");
                                    ret = RET_ERROR;
                                    for(++i; i < num_op; i++)
                                        vals[i] = NULL;
                                } else
                                    tmp_size++;
                            }
                            if(num_ret == 1 && tmp_size != 1)
                                num_ret = tmp_size;
                            else if(num_ret != tmp_size && tmp_size != 1) {
                                error("Runtime error: Modulo symmetry error.");
                                ret = RET_ERROR;
                                for(++i; i < num_op; i++)
                                    vals[i] = NULL;
                            }
                        }
                    }
                    if(ret != RET_ERROR) {
                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
                        for(int i = 0; i < num_ret; i++) {
                            long ret_part;
                            if(vals[0][1] == NULL) {
                                ret_part = (long)(vals[0][0]->data.number);
                            } else {
                                ret_part = (long)(vals[0][i]->data.number);
                            }

                            for(int j = 1; j < num_op; j++) {
                                if(vals[j][1] == NULL) {
                                    ret_part %= (long)(vals[j][0]->data.number);
                                } else {
                                    ret_part %= (long)(vals[j][i]->data.number);
                                }
                            }

                            ret[i] = object_create_number(ret_part);
                            object_reference(ret[i]);
                        }
                        ret[num_ret] = NULL;
                    }

                    for(int i = 0; i < num_op; i++) {
                        if(vals[i] != RET_ERROR && vals[i] != NULL) {
                            for(int j = 0; vals[i][j] != NULL; j++)
                                object_dereference(vals[i][j]);
                            _free(vals[i]);
                        }
                    }
                    _free(vals);
                } break;
                case OPERATION_TYPE_NEG: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Negate NULL error.");
                        ret = RET_ERROR;
                    } else if (vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Negate type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(-vals[i]->data.number);
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }
                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_POW: {
                    size_t num_op = 0;
                    while(op->data.operations[num_op] != NULL) num_op++;

                    object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

                    size_t num_ret = 1;
                    for(int i = 0; ret != RET_ERROR && i < num_op; i++) {
                        vals[i] = operation_result(op->data.operations[i], env);
                        if(vals[i] == NULL) {
                            error("Runtime error: Power NULL error.");
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else if (vals[i] == RET_ERROR) {
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else {
                            size_t tmp_size = 0;
                            while(ret != RET_ERROR && vals[i][tmp_size] != NULL) {
                                if(vals[i][tmp_size]->type != OBJECT_TYPE_NUMBER) {
                                    error("Runtime error: Power type error.");
                                    ret = RET_ERROR;
                                    for(++i; i < num_op; i++)
                                        vals[i] = NULL;
                                } else
                                    tmp_size++;
                            }
                            if(num_ret == 1 && tmp_size != 1)
                                num_ret = tmp_size;
                            else if(num_ret != tmp_size && tmp_size != 1) {
                                error("Runtime error: Power symmetry error.");
                                ret = RET_ERROR;
                                for(++i; i < num_op; i++)
                                    vals[i] = NULL;
                            }
                        }
                    }
                    if(ret != RET_ERROR) {
                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
                        for(int i = 0; i < num_ret; i++) {
                            number_t ret_part;
                            if(vals[num_op-1][1] == NULL)
                                ret_part = (int)(vals[num_op-1][0]->data.number);
                            else
                                ret_part = (int)(vals[num_op-1][i]->data.number);


                            for(int j = num_op-2; j >= 0; j--) {
                                if(vals[j][1] == NULL)
                                    ret_part = pow(vals[j][0]->data.number, ret_part);
                                else
                                    ret_part = pow(vals[j][i]->data.number, ret_part);
                            }

                            ret[i] = object_create_number(ret_part);
                            object_reference(ret[i]);
                        }
                        ret[num_ret] = NULL;
                    }

                    for(int i = 0; i < num_op; i++) {
                        if(vals[i] != RET_ERROR && vals[i] != NULL) {
                            for(int j = 0; vals[i][j] != NULL; j++)
                                object_dereference(vals[i][j]);
                            _free(vals[i]);
                        }
                    }
                    _free(vals);
                } break;
                case OPERATION_TYPE_AND: {
                    size_t num_op = 0;
                    while(op->data.operations[num_op] != NULL) num_op++;

                    object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

                    size_t num_ret = 1;
                    for(int i = 0; ret != RET_ERROR && i < num_op; i++) {
                        vals[i] = operation_result(op->data.operations[i], env);
                        if(vals[i] == NULL) {
                            error("Runtime error: And NULL error.");
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else if (vals[i] == RET_ERROR) {
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else {
                            size_t tmp_size = 0;
                            while(ret != RET_ERROR && vals[i][tmp_size] != NULL) {
                                if(vals[i][tmp_size]->type != OBJECT_TYPE_BOOL) {
                                    error("Runtime error: And type error.");
                                    ret = RET_ERROR;
                                    for(++i; i < num_op; i++)
                                        vals[i] = NULL;
                                } else
                                    tmp_size++;
                            }
                            if(num_ret == 1 && tmp_size != 1)
                                num_ret = tmp_size;
                            else if(num_ret != tmp_size && tmp_size != 1) {
                                error("Runtime error: And symmetry error.");
                                ret = RET_ERROR;
                                for(++i; i < num_op; i++)
                                    vals[i] = NULL;
                            }
                        }
                    }
                    if(ret != RET_ERROR) {
                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
                        for(int i = 0; i < num_ret; i++) {
                            bool_t ret_part = true;

                            for(int j = 0; j < num_op; j++) {
                                if(vals[j][1] == NULL)
                                    ret_part = ret_part && vals[j][0]->data.boolean;
                                else
                                    ret_part = ret_part && vals[j][i]->data.boolean;
                            }

                            ret[i] = object_create_boolean(ret_part);
                            object_reference(ret[i]);
                        }
                        ret[num_ret] = NULL;
                    }

                    for(int i = 0; i < num_op; i++) {
                        if(vals[i] != RET_ERROR && vals[i] != NULL) {
                            for(int j = 0; vals[i][j] != NULL; j++)
                                object_dereference(vals[i][j]);
                            _free(vals[i]);
                        }
                    }
                    _free(vals);
                } break;
                case OPERATION_TYPE_OR: {
                    size_t num_op = 0;
                    while(op->data.operations[num_op] != NULL) num_op++;

                    object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

                    size_t num_ret = 1;
                    for(int i = 0; ret != RET_ERROR && i < num_op; i++) {
                        vals[i] = operation_result(op->data.operations[i], env);
                        if(vals[i] == NULL) {
                            error("Runtime error: Or NULL error.");
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else if(vals[i] == RET_ERROR) {
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else {
                            size_t tmp_size = 0;
                            while(ret != RET_ERROR && vals[i][tmp_size] != NULL) {
                                if(vals[i][tmp_size]->type != OBJECT_TYPE_BOOL) {
                                    error("Runtime error: Or type error.");
                                    ret = RET_ERROR;
                                    for(++i; i < num_op; i++)
                                        vals[i] = NULL;
                                } else
                                    tmp_size++;
                            }
                            if(num_ret == 1 && tmp_size != 1)
                                num_ret = tmp_size;
                            else if(num_ret != tmp_size && tmp_size != 1) {
                                error("Runtime error: Or symmetry error.");
                                ret = RET_ERROR;
                                for(++i; i < num_op; i++)
                                    vals[i] = NULL;
                            }
                        }
                    }
                    if(ret != RET_ERROR) {
                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
                        for(int i = 0; i < num_ret; i++) {
                            bool_t ret_part = false;

                            for(int j = 0; j < num_op; j++) {
                                if(vals[j][1] == NULL)
                                    ret_part = ret_part || vals[j][0]->data.boolean;
                                else
                                    ret_part = ret_part || vals[j][i]->data.boolean;
                            }

                            ret[i] = object_create_boolean(ret_part);
                            object_reference(ret[i]);
                        }
                        ret[num_ret] = NULL;
                    }

                    for(int i = 0; i < num_op; i++) {
                        if(vals[i] != RET_ERROR && vals[i] != NULL) {
                            for(int j = 0; vals[i][j] != NULL; j++)
                                object_dereference(vals[i][j]);
                            _free(vals[i]);
                        }
                    }
                    _free(vals);
                } break;
                case OPERATION_TYPE_XOR: {
                    size_t num_op = 0;
                    while(op->data.operations[num_op] != NULL) num_op++;

                    object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

                    size_t num_ret = 1;
                    for(int i = 0; ret != RET_ERROR && i < num_op; i++) {
                        vals[i] = operation_result(op->data.operations[i], env);
                        if(vals[i] == NULL) {
                            error("Runtime error: Xor NULL error.");
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else if(vals[i] == RET_ERROR) {
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else {
                            size_t tmp_size = 0;
                            while(ret != RET_ERROR && vals[i][tmp_size] != NULL) {
                                if(vals[i][tmp_size]->type != OBJECT_TYPE_BOOL) {
                                    error("Runtime error: Xor type error.");
                                    ret = RET_ERROR;
                                    for(++i; i < num_op; i++)
                                        vals[i] = NULL;
                                } else
                                    tmp_size++;
                            }
                            if(num_ret == 1 && tmp_size != 1)
                                num_ret = tmp_size;
                            else if(num_ret != tmp_size && tmp_size != 1) {
                                error("Runtime error: Xor symmetry error.");
                                ret = RET_ERROR;
                                for(++i; i < num_op; i++)
                                    vals[i] = NULL;
                            }
                        }
                    }
                    if(ret != RET_ERROR) {
                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
                        for(int i = 0; i < num_ret; i++) {
                            bool_t ret_part = false;

                            for(int j = 0; j < num_op; j++) {
                                if(vals[j][1] == NULL)
                                    ret_part = ret_part != vals[j][0]->data.boolean;
                                else
                                    ret_part = ret_part != vals[j][i]->data.boolean;
                            }

                            ret[i] = object_create_boolean(ret_part);
                            object_reference(ret[i]);
                        }
                        ret[num_ret] = NULL;
                    }

                    for(int i = 0; i < num_op; i++) {
                        if(vals[i] != RET_ERROR && vals[i] != NULL) {
                            for(int j = 0; vals[i][j] != NULL; j++)
                                object_dereference(vals[i][j]);
                            _free(vals[i]);
                        }
                    }
                    _free(vals);
                } break;
                case OPERATION_TYPE_NOT: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Not NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_BOOL) {
                                error("Runtime error: Not type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_boolean(!vals[i]->data.boolean);
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }
                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_SQRT: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Sqrt NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Sqrt type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(sqrt(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }
                        for(int i = 0; vals[i] != NULL; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_CBRT: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Cbrt NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Cbrt type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(cbrt(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_SIN: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Sin NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Sin type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(sin(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_COS: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Cos NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Cos type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(cos(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_TAN: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Tan NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Tan type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(tan(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_ASIN: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Asin NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Asin type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(asin(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_ACOS: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Acos NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Acos type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(acos(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_ATAN: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Atan NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Atan type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(atan(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }

                } break;
                case OPERATION_TYPE_SINH: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Sinh NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Sinh type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(sinh(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_COSH: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Cosh NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Cosh type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(cosh(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_TANH: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Tanh NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Tanh type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(tanh(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_ASINH: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Asinh NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Asinh type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(asinh(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_ACOSH: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Acosh NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Acosh type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(acosh(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_ATANH: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Atanh NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Atanh type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(atanh(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_TRUNC: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Trunc NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Trunc type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(trunc(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_FLOOR: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Floor NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Floor type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(floor(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_CEIL: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Ceil NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Ceil type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(ceil(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_ROUND: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Round NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Round type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(round(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_RAND:
                    ret = (object_t**)_alloc(sizeof(object_t*)*2);
                    ret[0] = object_create_number((double)rand()/RAND_MAX);
                    object_reference(ret[0]);
                    ret[1] = NULL;
                    break;
                case OPERATION_TYPE_LEN: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Len NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(vals[num_vals] != NULL) {
                            num_vals++;
                        }

                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                        for(int i = 0; ret != RET_ERROR && i < num_vals; i++) {
                            if(vals[i]->type == OBJECT_TYPE_LIST) {
                                ret[i] = object_create_number(list_size(vals[i]->data.list));
                            } else if (vals[i]->type == OBJECT_TYPE_STRING) {
                                ret[i] = object_create_number(string_length(vals[i]->data.string));
                            } else {
                                error("Runtime error: Len type error.");
                                for(int j = 0; j < i; j++)
                                    object_dereference(ret[j]);
                                _free(ret);
                                ret = RET_ERROR;
                            }
                            if(ret != RET_ERROR)
                                object_reference(ret[i]);
                        }
                        if(ret != RET_ERROR)
                            ret[num_vals] = NULL;

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_EQU: {
                    object_t** left = operation_result(op->data.operations[0], env);
                    object_t** right = operation_result(op->data.operations[1], env);

                    if(left == NULL || right == NULL) {
                        error("Runtime error: Equ NULL error.");
                        ret = RET_ERROR;
                    } else if(left == RET_ERROR || right == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t left_len = 0;
                        while(left[left_len] != NULL) left_len++;
                        size_t right_len = 0;
                        while(right[right_len] != NULL) right_len++;

                        if(left_len > 1 && right_len > 1) {
                            if(left_len != right_len) {
                                error("Runtime error: Equ symmetry error");
                                ret = RET_ERROR;
                            } else {
                                ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

                                for(int i = 0; i < right_len; i++) {
                                    ret[i] = object_create_boolean(object_equ(left[i], right[i]));
                                    object_reference(ret[i]);
                                }
                                ret[right_len] = NULL;
                            }
                        } else if(left_len < right_len) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

                            for(int i = 0; i < right_len; i++) {
                                ret[i] = object_create_boolean(object_equ(left[0], right[i]));
                                object_reference(ret[i]);
                            }
                            ret[right_len] = NULL;
                        } else {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(left_len + 1));

                            for(int i = 0; i < left_len; i++) {
                                ret[i] = object_create_boolean(object_equ(left[i], right[0]));
                                object_reference(ret[i]);
                            }
                            ret[left_len] = NULL;
                        }
                    }

                    if(left != RET_ERROR && left != NULL) {
                        for(int i = 0; left[i] != NULL; i++)
                            object_dereference(left[i]);
                        _free(left);
                    }
                    if(right != RET_ERROR && right != NULL) {
                        for(int i = 0; right[i] != NULL; i++)
                            object_dereference(right[i]);
                        _free(right);
                    }
                } break;
                case OPERATION_TYPE_GEQ: {
                    object_t** left = operation_result(op->data.operations[0], env);
                    object_t** right = operation_result(op->data.operations[1], env);

                    if(left == NULL || right == NULL) {
                        error("Runtime error: Compare NULL error.");
                        ret = RET_ERROR;
                    } else if(left == RET_ERROR || right == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t left_len = 0;
                        while(left[left_len] != NULL) left_len++;
                        size_t right_len = 0;
                        while(right[right_len] != NULL) right_len++;

                        if(left_len > 1 && right_len > 1) {
                            if(left_len != right_len) {
                                error("Runtime error: Compare symmetry error");
                                ret = RET_ERROR;
                            }
                            else {
                                ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

                                for(int i = 0; ret != RET_ERROR && i < right_len; i++) {
                                    if(left[i]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER) {
                                        error("Runtime error: Compare type error");
                                        for(int j = 0; j < i; j++)
                                            object_dereference(ret[j]);
                                        _free(ret);
                                        ret = RET_ERROR;
                                    } else {
                                        ret[i] = object_create_boolean(left[i]->data.number >= right[i]->data.number);
                                        object_reference(ret[i]);
                                    }
                                }
                                if(ret != RET_ERROR)
                                    ret[right_len] = NULL;
                            }
                        } else if(left_len < right_len) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

                            for(int i = 0; ret != RET_ERROR && i < right_len; i++) {
                                if(left[0]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER) {
                                    error("Runtime error: Compare type error");
                                    for(int j = 0; j < i; j++)
                                        object_dereference(ret[j]);
                                    _free(ret);
                                    ret = RET_ERROR;
                                } else {
                                    ret[i] = object_create_boolean(left[0]->data.number >= right[i]->data.number);
                                    object_reference(ret[i]);
                                }
                            }
                            ret[right_len] = NULL;
                        } else {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(left_len + 1));

                            for(int i = 0; ret != RET_ERROR && i < left_len; i++) {
                                if(left[i]->type != OBJECT_TYPE_NUMBER || right[0]->type != OBJECT_TYPE_NUMBER) {
                                    error("Runtime error: Compare type error");
                                    for(int j = 0; j < i; j++)
                                        object_dereference(ret[j]);
                                    _free(ret);
                                    ret = RET_ERROR;
                                } else {
                                    ret[i] = object_create_boolean(left[i]->data.number >= right[0]->data.number);
                                    object_reference(ret[i]);
                                }
                            }
                            ret[left_len] = NULL;
                        }
                    }

                    if(left != RET_ERROR && left != NULL) {
                        for(int i = 0; left[i] != NULL; i++)
                            object_dereference(left[i]);
                        _free(left);
                    }
                    if(right != RET_ERROR && right != NULL) {
                        for(int i = 0; right[i] != NULL; i++)
                            object_dereference(right[i]);
                        _free(right);
                    }
                } break;
                case OPERATION_TYPE_LEQ: {
                    object_t** left = operation_result(op->data.operations[0], env);
                    object_t** right = operation_result(op->data.operations[1], env);

                    if(left == NULL || right == NULL) {
                        error("Runtime error: Compare NULL error.");
                        ret = RET_ERROR;
                    } else if(left == RET_ERROR || right == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t left_len = 0;
                        while(left[left_len] != NULL) left_len++;
                        size_t right_len = 0;
                        while(right[right_len] != NULL) right_len++;

                        if(left_len > 1 && right_len > 1) {
                            if(left_len != right_len) {
                                error("Runtime error: Compare symmetry error");
                                ret = RET_ERROR;
                            }
                            else {
                                ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

                                for(int i = 0; ret != RET_ERROR && i < right_len; i++) {
                                    if(left[i]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER) {
                                        error("Runtime error: Compare type error");
                                        for(int j = 0; j < i; j++)
                                            object_dereference(ret[j]);
                                        _free(ret);
                                        ret = RET_ERROR;
                                    } else {
                                        ret[i] = object_create_boolean(left[i]->data.number <= right[i]->data.number);
                                        object_reference(ret[i]);
                                    }
                                }
                                if(ret != RET_ERROR)
                                    ret[right_len] = NULL;
                            }
                        } else if(left_len < right_len) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

                            for(int i = 0; ret != RET_ERROR && i < right_len; i++) {
                                if(left[0]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER) {
                                    error("Runtime error: Compare type error");
                                    for(int j = 0; j < i; j++)
                                        object_dereference(ret[j]);
                                    _free(ret);
                                    ret = RET_ERROR;
                                } else {
                                    ret[i] = object_create_boolean(left[0]->data.number <= right[i]->data.number);
                                    object_reference(ret[i]);
                                }
                            }
                            ret[right_len] = NULL;
                        } else {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(left_len + 1));

                            for(int i = 0; ret != RET_ERROR && i < left_len; i++) {
                                if(left[i]->type != OBJECT_TYPE_NUMBER || right[0]->type != OBJECT_TYPE_NUMBER) {
                                    error("Runtime error: Compare type error");
                                    for(int j = 0; j < i; j++)
                                        object_dereference(ret[j]);
                                    _free(ret);
                                    ret = RET_ERROR;
                                } else {
                                    ret[i] = object_create_boolean(left[i]->data.number <= right[0]->data.number);
                                    object_reference(ret[i]);
                                }
                            }
                            ret[left_len] = NULL;
                        }
                    }

                    if(left != RET_ERROR && left != NULL) {
                        for(int i = 0; left[i] != NULL; i++)
                            object_dereference(left[i]);
                        _free(left);
                    }
                    if(right != RET_ERROR && right != NULL) {
                        for(int i = 0; right[i] != NULL; i++)
                            object_dereference(right[i]);
                        _free(right);
                    }
                } break;
                case OPERATION_TYPE_GTR: {
                    object_t** left = operation_result(op->data.operations[0], env);
                    object_t** right = operation_result(op->data.operations[1], env);

                    if(left == NULL || right == NULL) {
                        error("Runtime error: Compare NULL error.");
                        ret = RET_ERROR;
                    } else if(left == RET_ERROR || right == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t left_len = 0;
                        while(left[left_len] != NULL) left_len++;
                        size_t right_len = 0;
                        while(right[right_len] != NULL) right_len++;

                        if(left_len > 1 && right_len > 1) {
                            if(left_len != right_len) {
                                error("Runtime error: Compare symmetry error");
                                ret = RET_ERROR;
                            }
                            else {
                                ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

                                for(int i = 0; ret != RET_ERROR && i < right_len; i++) {
                                    if(left[i]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER) {
                                        error("Runtime error: Compare type error");
                                        for(int j = 0; j < i; j++)
                                            object_dereference(ret[j]);
                                        _free(ret);
                                        ret = RET_ERROR;
                                    } else {
                                        ret[i] = object_create_boolean(left[i]->data.number > right[i]->data.number);
                                        object_reference(ret[i]);
                                    }
                                }
                                if(ret != RET_ERROR)
                                    ret[right_len] = NULL;
                            }
                        } else if(left_len < right_len) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

                            for(int i = 0; ret != RET_ERROR && i < right_len; i++) {
                                if(left[0]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER) {
                                    error("Runtime error: Compare type error");
                                    for(int j = 0; j < i; j++)
                                        object_dereference(ret[j]);
                                    _free(ret);
                                    ret = RET_ERROR;
                                } else {
                                    ret[i] = object_create_boolean(left[0]->data.number > right[i]->data.number);
                                    object_reference(ret[i]);
                                }
                            }
                            ret[right_len] = NULL;
                        } else {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(left_len + 1));

                            for(int i = 0; ret != RET_ERROR && i < left_len; i++) {
                                if(left[i]->type != OBJECT_TYPE_NUMBER || right[0]->type != OBJECT_TYPE_NUMBER) {
                                    error("Runtime error: Compare type error");
                                    for(int j = 0; j < i; j++)
                                        object_dereference(ret[j]);
                                    _free(ret);
                                    ret = RET_ERROR;
                                } else {
                                    ret[i] = object_create_boolean(left[i]->data.number > right[0]->data.number);
                                    object_reference(ret[i]);
                                }
                            }
                            ret[left_len] = NULL;
                        }
                    }

                    if(left != RET_ERROR && left != NULL) {
                        for(int i = 0; left[i] != NULL; i++)
                            object_dereference(left[i]);
                        _free(left);
                    }
                    if(right != RET_ERROR && right != NULL) {
                        for(int i = 0; right[i] != NULL; i++)
                            object_dereference(right[i]);
                        _free(right);
                    }
                } break;
                case OPERATION_TYPE_LES: {
                    object_t** left = operation_result(op->data.operations[0], env);
                    object_t** right = operation_result(op->data.operations[1], env);

                    if(left == NULL || right == NULL) {
                        error("Runtime error: Compare NULL error.");
                        ret = RET_ERROR;
                    } else if(left == RET_ERROR || right == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t left_len = 0;
                        while(left[left_len] != NULL) left_len++;
                        size_t right_len = 0;
                        while(right[right_len] != NULL) right_len++;

                        if(left_len > 1 && right_len > 1) {
                            if(left_len != right_len) {
                                error("Runtime error: Compare symmetry error");
                                ret = RET_ERROR;
                            }
                            else {
                                ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

                                for(int i = 0; ret != RET_ERROR && i < right_len; i++) {
                                    if(left[i]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER) {
                                        error("Runtime error: Compare type error");
                                        for(int j = 0; j < i; j++)
                                            object_dereference(ret[j]);
                                        _free(ret);
                                        ret = RET_ERROR;
                                    } else {
                                        ret[i] = object_create_boolean(left[i]->data.number < right[i]->data.number);
                                        object_reference(ret[i]);
                                    }
                                }
                                if(ret != RET_ERROR)
                                    ret[right_len] = NULL;
                            }
                        } else if(left_len < right_len) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

                            for(int i = 0; ret != RET_ERROR && i < right_len; i++) {
                                if(left[0]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER) {
                                    error("Runtime error: Compare type error");
                                    for(int j = 0; j < i; j++)
                                        object_dereference(ret[j]);
                                    _free(ret);
                                    ret = RET_ERROR;
                                } else {
                                    ret[i] = object_create_boolean(left[0]->data.number < right[i]->data.number);
                                    object_reference(ret[i]);
                                }
                            }
                            ret[right_len] = NULL;
                        } else {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(left_len + 1));

                            for(int i = 0; ret != RET_ERROR && i < left_len; i++) {
                                if(left[i]->type != OBJECT_TYPE_NUMBER || right[0]->type != OBJECT_TYPE_NUMBER) {
                                    error("Runtime error: Compare type error");
                                    for(int j = 0; j < i; j++)
                                        object_dereference(ret[j]);
                                    _free(ret);
                                    ret = RET_ERROR;
                                } else {
                                    ret[i] = object_create_boolean(left[i]->data.number < right[0]->data.number);
                                    object_reference(ret[i]);
                                }
                            }
                            ret[left_len] = NULL;
                        }
                    }

                    if(left != RET_ERROR && left != NULL) {
                        for(int i = 0; left[i] != NULL; i++)
                            object_dereference(left[i]);
                        _free(left);
                    }
                    if(right != RET_ERROR && right != NULL) {
                        for(int i = 0; right[i] != NULL; i++)
                            object_dereference(right[i]);
                        _free(right);
                    }
                } break;
                case OPERATION_TYPE_DIC: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        dictionary_t* dic;
                        if(vals != NULL) {
                            size_t num_vals = 0;
                            while(vals[num_vals] != NULL) num_vals++;

                            dic = dictionary_create_sized(num_vals*2);

                            for(int i = 0; ret != RET_ERROR && i < num_vals; i++) {
                                if(vals[i]->type != OBJECT_TYPE_PAIR) {
                                    error("Runtime error: Dictionary type error.");
                                    dictionary_free(dic);
                                    ret = RET_ERROR;
                                } else
                                    dictionary_put(dic, vals[i]->data.pair->key, vals[i]->data.pair->value);
                            }
                            for(int i = 0; i < num_vals; i++)
                                object_dereference(vals[i]);
                            _free(vals);
                        } else {
                            dic = dictionary_create();
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*2);
                            ret[0] = object_create_dictionary(dic);
                            object_reference(ret[0]);
                            ret[1] = NULL;
                        }
                    }
                } break;
                case OPERATION_TYPE_FIND: {
                    object_t** left = operation_result(op->data.operations[0], env);
                    object_t** right = operation_result(op->data.operations[1], env);

                    if(left == NULL || right == NULL) {
                        error("Runtime error: Find NULL error.");
                    } else if(left == RET_ERROR || right == RET_ERROR) {

                    } else {
                        size_t left_len = 0;
                        while(left[left_len] != NULL) left_len++;
                        size_t right_len = 0;
                        while(right[right_len] != NULL) right_len++;

                        ret = (object_t**)_alloc(sizeof(object_t*)*(left_len*right_len+1));
                        for(int i = 0; ret != RET_ERROR && i < left_len; i++)
                            for(int j = 0; ret != RET_ERROR && j < right_len; j++) {
                                switch(left[i]->type) {
                                    case OBJECT_TYPE_LIST: {
                                        pos_t pos = list_find(left[i]->data.list, right[j]);
                                        if(pos != -1)
                                            ret[i*right_len+j] = object_create_number((number_t)pos);
                                        else
                                            ret[i*right_len+j] = object_create_none();
                                    } break;
                                    case OBJECT_TYPE_STRING:
                                        if(right[j]->type != OBJECT_TYPE_STRING) {
                                            error("Runtime error: Find type error.");
                                            for(int k = 0; k < i*right_len+j; k++)
                                                object_reference(ret[k]);
                                            _free(ret);
                                            ret = RET_ERROR;
                                        } else {
                                            pos_t pos = string_find(left[i]->data.string, right[j]->data.string);
                                            if(pos != -1)
                                                ret[i*right_len+j] = object_create_number((number_t)pos);
                                            else
                                                ret[i*right_len+j] = object_create_none();
                                        }
                                        break;
                                    case OBJECT_TYPE_DICTIONARY: ret[i*right_len+j] = object_create_boolean(dictionary_get(left[i]->data.dic, right[j]) != NULL); break;
                                    default:
                                        error("Runtime error: Find type error.");
                                        for(int k = 0; k < i*right_len+j; k++)
                                            object_reference(ret[k]);
                                        _free(ret);
                                        ret = RET_ERROR;
                                        break;
                                }
                                if(ret != RET_ERROR)
                                    object_reference(ret[i*right_len+j]);
                            }
                        if(ret != RET_ERROR)
                            ret[left_len*right_len] = NULL;
                    }

                    if(left != RET_ERROR && left != NULL) {
                        for(int i = 0; left[i] != NULL; i++)
                            object_dereference(left[i]);
                        _free(left);
                    }
                    if(right != RET_ERROR && right != NULL) {
                        for(int i = 0; right[i] != NULL; i++)
                            object_dereference(right[i]);
                        _free(right);
                    }
                } break;
                case OPERATION_TYPE_SPLIT: {
                    object_t** src = operation_result(op->data.operations[0], env);
                    object_t** splt = operation_result(op->data.operations[1], env);

                    if(src == NULL || splt == NULL) {
                        error("Runtime error: Split NULL error.");
                        ret = RET_ERROR;
                    } else if(src == RET_ERROR || splt == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t src_len = 0;
                        while(ret != RET_ERROR && src[src_len] != NULL) {
                            if(src[src_len]->type != OBJECT_TYPE_STRING) {
                                error("Runtime error: Split type error.");
                                ret = RET_ERROR;
                            } else
                                src_len++;
                        }
                        size_t splt_len = 0;
                        if(ret != RET_ERROR) {
                            while(ret != RET_ERROR && splt[splt_len] != NULL) {
                                if(splt[splt_len]->type != OBJECT_TYPE_STRING) {
                                    error("Runtime error: Split type error.");
                                    ret = RET_ERROR;
                                } else
                                    splt_len++;
                            }
                        }
                        if(ret != RET_ERROR) {
                            size_t num_ret = 0;
                            for(int i = 0; i < src_len; i++) {
                                pos_t last_pos = 0;
                                while(last_pos != -1) {
                                    pos_t min_pos = -1;
                                    size_t len = 0;
                                    for(int j = 0; j < splt_len; j++) {
                                        pos_t tmp = string_find_from(src[i]->data.string, splt[j]->data.string, last_pos);
                                        if((min_pos == -1 || tmp < min_pos) && tmp != -1)
                                            min_pos = tmp, len = string_length(splt[j]->data.string);
                                    }
                                    last_pos = min_pos + len;
                                    num_ret++;
                                }
                            }
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));

                            upos_t index = 0;
                            for(int i = 0; i < src_len; i++) {
                                pos_t last_pos = 0;
                                while(last_pos != -1) {
                                    pos_t min_pos = -1;
                                    size_t len = 0;
                                    for(int j = 0; j < splt_len; j++) {
                                        pos_t tmp = string_find_from(src[i]->data.string, splt[j]->data.string, last_pos);
                                        if((min_pos == -1 || tmp < min_pos) && tmp != -1)
                                            min_pos = tmp, len = string_length(splt[j]->data.string);
                                    }
                                    ret[index] = object_create_string(string_substr(src[i]->data.string, last_pos,
                                                                    (min_pos == -1 ? string_length(src[i]->data.string) - last_pos: min_pos - last_pos)));
                                    object_reference(ret[index]);
                                    last_pos = min_pos + len;
                                    index++;
                                }
                            }
                            ret[num_ret] = NULL;
                        }
                    }

                    if(src != RET_ERROR && src != NULL) {
                        for(int i = 0; src[i] != NULL; i++)
                            object_dereference(src[i]);
                        _free(src);
                    }
                    if(splt != RET_ERROR && splt != NULL) {
                        for(int i = 0; splt[i] != NULL; i++)
                            object_dereference(splt[i]);
                        _free(splt);
                    }
                } break;
                case OPERATION_TYPE_ABS: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Abs NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(ret != RET_ERROR && vals[num_vals] != NULL) {
                            if(vals[num_vals]->type != OBJECT_TYPE_NUMBER) {
                                error("Runtime error: Abs type error.");
                                ret = RET_ERROR;
                            } else
                                num_vals++;
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                            for(int i = 0; i < num_vals; i++) {
                                ret[i] = object_create_number(abs(vals[i]->data.number));
                                object_reference(ret[i]);
                            }
                            ret[num_vals] = NULL;
                        }

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_SCOPE:
                    environment_add_scope(env);
                    ret = operation_result(op->data.operations[0], env);
                    environment_remove_scope(env);
                break;
                case OPERATION_TYPE_IF: {
                    object_t** cond = operation_result(op->data.operations[0], env);

                    if(cond == NULL) {
                        error("Runtime error: If NULL error.");
                        ret = RET_ERROR;
                    } else if(cond == RET_ERROR) {
                        ret = RET_ERROR;
                    } else if(cond[1] != NULL) {
                        error("Runtime error: If non-scalar condition error.");
                        ret = RET_ERROR;
                    } else {
                        if(is_true(cond[0]))
                            ret = operation_result(op->data.operations[1], env);
                    }

                    if(cond != RET_ERROR && cond != NULL) {
                        for(int i = 0; cond[i] != NULL; i++)
                            object_dereference(cond[i]);
                        _free(cond);
                    }
                } break;
                case OPERATION_TYPE_IFELSE: {
                    object_t** cond = operation_result(op->data.operations[0], env);

                    if(cond == NULL) {
                        error("Runtime error: If-else NULL error.");
                        ret = RET_ERROR;
                    } else if(cond == RET_ERROR) {
                        ret = RET_ERROR;
                    } else if(cond[1] != NULL) {
                        error("Runtime error: If-else non-scalar condition error.");
                        ret = RET_ERROR;
                    } else {
                        if(is_true(cond[0]))
                            ret = operation_result(op->data.operations[1], env);
                        else
                            ret = operation_result(op->data.operations[2], env);
                    }

                    if(cond != RET_ERROR && cond != NULL) {
                        for(int i = 0; cond[i] != NULL; i++)
                            object_dereference(cond[i]);
                        _free(cond);
                    }
                } break;
                case OPERATION_TYPE_WHILE: {
                    list_t* list = list_create_empty();

                    object_t** cond = operation_result(op->data.operations[0], env);

                    if(cond == NULL) {
                        error("Runtime error: While NULL error.");
                        ret = RET_ERROR;
                    } else if(cond == RET_ERROR) {
                        ret = RET_ERROR;
                    } else if(cond[1] != NULL) {
                        error("Runtime error: While non-scalar condition error.");
                        ret = RET_ERROR;
                    } else
                        while(ret != RET_ERROR && is_true(cond[0]))
                        {
                            object_t** tmp = operation_result(op->data.operations[1], env);
                            if(tmp == NULL) {
                                error("Runtime error: While NULL error.");
                                list_free(list);
                                ret = RET_ERROR;
                            } else if(tmp == RET_ERROR) {
                                list_free(list);
                                ret = RET_ERROR;
                            } else {
                                for(int i = 0; tmp[i] != NULL; i++) {
                                    list_append(list, tmp[i]);
                                    object_dereference(tmp[i]);
                                }
                                _free(tmp);

                                object_dereference(cond[0]);
                                _free(cond);
                                cond = operation_result(op->data.operations[0], env);
                                if(cond == NULL) {
                                    error("Runtime error: While NULL error.");
                                    list_free(list);
                                    ret = RET_ERROR;
                                } else if(cond == RET_ERROR) {
                                    list_free(list);
                                    ret = RET_ERROR;
                                } else if(cond[1] != NULL) {
                                    error("Runtime error: While non-scalar condition error.");
                                    list_free(list);
                                    ret = RET_ERROR;
                                }
                            }
                        }

                    if(cond != RET_ERROR && cond != NULL) {
                        for(int i = 0; cond[i] != NULL; i++)
                            object_dereference(cond[i]);
                        _free(cond);
                    }

                    if(ret != RET_ERROR) {
                        list_append(list, NULL);
                        ret = list->data;
                        _free(list);
                    }
                } break;
                case OPERATION_TYPE_IN_STRUCT: {
                    object_t** stc = operation_result(op->data.operations[0], env);

                    if(stc == NULL) {
                        error("Runtime error: Struct NULL error.");
                        ret = RET_ERROR;
                    } else if(stc == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_stc = 0;
                        while(stc[num_stc] != NULL) num_stc++;

                        object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_stc);

                        size_t num_ret = 0;
                        for(int i = 0; ret != RET_ERROR && i < num_stc; i++) {
                            if(stc[i]->type != OBJECT_TYPE_STRUCT) {
                                error("Runtime error: Struct type error.");
                                ret = RET_ERROR;
                                for(; i < num_stc; i++)
                                    vals[i] = NULL;
                            } else {
                                vals[i] = struct_result(stc[i]->data.stc, op->data.operations[1]);
                                if(vals[i] == NULL) {
                                    error("Runtime error: Struct NULL error");
                                    ret = RET_ERROR;
                                    for(++i; i < num_stc; i++)
                                        vals[i] = NULL;
                                } else if(vals[i] == RET_ERROR) {
                                    ret = RET_ERROR;
                                    for(++i; i < num_stc; i++)
                                        vals[i] = NULL;
                                } else
                                    for(int j = 0; vals[i][j] != NULL; j++)
                                        num_ret++;
                            }
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
                            num_ret = 0;
                            for(int i = 0; i < num_stc; i++) {
                                for(int j = 0; vals[i][j] != NULL; j++) {
                                    ret[num_ret] = vals[j][i];
                                    num_ret++;
                                }
                            }
                            ret[num_ret] = NULL;
                        }

                        for(int i = 0; i < num_stc; i++)
                            if(ret != RET_ERROR) {
                                _free(vals[i]);
                            } else {
                                if(vals[i] != RET_ERROR && vals[i] != NULL) {
                                    for(int j = 0; vals[i][j] != NULL; j++)
                                        object_dereference(vals[i][j]);
                                    _free(vals[i]);
                                }
                            }
                        _free(vals);
                        for(int i = 0; i < num_stc; i++)
                            object_dereference(stc[i]);
                        _free(stc);
                    }
                } break;
                case OPERATION_TYPE_LOCAL: {
                    size_t prev_limit = env->local_mode_limit;
                    environment_set_local_mode(env, env->count-1);
                    ret = operation_result(op->data.operations[0], env);
                    environment_set_local_mode(env, prev_limit);
                } break;
                case OPERATION_TYPE_GLOBAL: {
                    size_t prev_limit = env->local_mode_limit;
                    environment_set_local_mode(env, 0);
                    ret = operation_result(op->data.operations[0], env);
                    environment_set_local_mode(env, prev_limit);
                } break;
                case OPERATION_TYPE_COPY: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Copy NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_vals = 0;
                        while(vals[num_vals] != NULL) num_vals++;

                        ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
                        for(int i = 0; ret != RET_ERROR && i < num_vals; i++) {
                            switch(vals[i]->type) {
                                case OBJECT_TYPE_BOOL: ret[i] = object_create_boolean(vals[i]->data.boolean); break;
                                case OBJECT_TYPE_DICTIONARY: ret[i] = object_create_dictionary(dictionary_copy(vals[i]->data.dic)); break;
                                case OBJECT_TYPE_FUNCTION: ret[i] = object_create_function(vals[i]->data.func); break;
                                case OBJECT_TYPE_LIST: ret[i] = object_create_list(list_copy(vals[i]->data.list)); break;
                                case OBJECT_TYPE_MACRO: ret[i] = object_create_macro(vals[i]->data.mac); break;
                                case OBJECT_TYPE_NONE: ret[i] = object_create_none(); break;
                                case OBJECT_TYPE_NUMBER: ret[i] = object_create_number(vals[i]->data.number); break;
                                case OBJECT_TYPE_PAIR: ret[i] = object_create_pair(pair_copy(vals[i]->data.pair)); break;
                                case OBJECT_TYPE_STRING: ret[i] = object_create_string(string_copy(vals[i]->data.string)); break;
                                case OBJECT_TYPE_STRUCT:
                                    error("Runtime error: Copy type error.");
                                    for(int j = 0; j < i; j++)
                                        object_dereference(ret[j]);
                                    _free(ret);
                                    ret = RET_ERROR;
                                    break;
                            }
                            if(ret != RET_ERROR)
                                object_reference(ret[i]);
                        }
                        if(ret != RET_ERROR)
                            ret[num_vals] = NULL;

                        for(int i = 0; i < num_vals; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_FOR: {
                    list_t* list = list_create_empty();

                    if(operation_exec(op->data.operations[0], env) == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        object_t** cond = operation_result(op->data.operations[1], env);

                        if(cond == NULL) {
                            error("Runtime error: For NULL error.");
                            ret = RET_ERROR;
                        } else if(cond == RET_ERROR) {
                            ret = RET_ERROR;
                        } else if(cond[1] != NULL) {
                            error("Runtime error: For non-scalar condition error.");
                            ret = RET_ERROR;
                        } else
                            while(ret != RET_ERROR && is_true(cond[0]))
                            {
                                object_t** tmp = operation_result(op->data.operations[3], env);
                                if(tmp == NULL) {
                                    error("Runtime error: For NULL error.");
                                    list_free(list);
                                    ret = RET_ERROR;
                                } else if(tmp == RET_ERROR) {
                                    list_free(list);
                                    ret = RET_ERROR;
                                } else if(operation_exec(op->data.operations[2], env) == RET_ERROR) {
                                    ret = RET_ERROR;
                                } else {
                                    for(int i = 0; tmp[i] != NULL; i++) {
                                        list_append(list, tmp[i]);
                                        object_dereference(tmp[i]);
                                    }
                                    _free(tmp);

                                    object_dereference(cond[1]);
                                    _free(cond);
                                    cond = operation_result(op->data.operations[1], env);
                                    if(cond == NULL) {
                                        error("Runtime error: For NULL error.");
                                        list_free(list);
                                        ret = RET_ERROR;
                                    } else if(cond == RET_ERROR) {
                                        list_free(list);
                                        ret = RET_ERROR;
                                    } else if(cond[1] != NULL) {
                                        error("Runtime error: For non-scalar condition error.");
                                        list_free(list);
                                        ret = RET_ERROR;
                                    }
                                }
                            }

                        if(cond != RET_ERROR && cond != NULL) {
                            for(int i = 0; cond[i] != NULL; i++)
                                object_dereference(cond[i]);
                            _free(cond);
                        }

                        if(ret != RET_ERROR) {
                            list_append(list, NULL);
                            ret = list->data;
                            _free(list);
                        }
                    }
                } break;
                case OPERATION_TYPE_LIST_OPEN: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: List-opening NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_ret = 0;

                        for(int i = 0; ret != RET_ERROR && vals[i] != NULL; i++)
                            if(vals[i]->type == OBJECT_TYPE_LIST) {
                                num_ret += vals[i]->data.list->size;
                            } else {
                                error("Runtime error: List-opening type error.");
                                ret = RET_ERROR;
                            }

                        if(ret != RET_ERROR) {
                            ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));

                            num_ret = 0;
                            for(int i = 0; vals[i] != NULL; i++)
                                for(int j = 0; j < vals[i]->data.list->size; j++) {
                                    ret[num_ret] = vals[i]->data.list->data[j];
                                    object_reference(ret[num_ret]);
                                    num_ret++;
                                }

                            ret[num_ret] = NULL;
                        }

                        for(int i = 0; vals[i] != NULL; i++)
                            object_dereference(vals[i]);
                        _free(vals);
                    }
                } break;
                case OPERATION_TYPE_FOR_IN: {
                    list_t* list = list_create_empty();

                    object_t*** vals_loc = operation_var(op->data.operations[0], env);
                    object_t** vals_in = operation_result(op->data.operations[1], env);

                    if(vals_loc == NULL || vals_in == NULL) {
                        error("Runtime error: For-in NULL error.");
                        ret = RET_ERROR;
                    } else if(vals_loc == RET_ERROR || vals_in == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        int pos_in = 0;
                        while(ret != RET_ERROR && vals_in[pos_in] != NULL) {
                            // Assign values
                            for(int i = 0; vals_loc[i] != NULL && vals_in[pos_in] != NULL; i++) {
                                if(vals_loc[i] == OBJECT_LIST_OPENED) {
                                    object_t** obj = vals_loc[i+1];
                                    object_dereference(*obj);

                                    size_t length_left = 0;
                                    while(vals_in[pos_in + length_left] != NULL) length_left++;

                                    list_t* list = list_create_null(length_left);
                                    for(int j = 0; j < length_left; j++) {
                                        list->data[j] = vals_in[i+j];
                                        object_reference(list->data[j]);
                                    }
                                    *obj = object_create_list(list);
                                    object_reference(*obj);
                                    pos_in += length_left;
                                } else {
                                    object_dereference(*(vals_loc[i]));
                                    *(vals_loc[i]) = vals_in[pos_in];
                                    object_reference(*(vals_loc[i]));
                                    pos_in++;
                                }
                            }

                            object_t** tmp = operation_result(op->data.operations[2], env);
                            if(tmp == NULL) {
                                error("Runtime error: For NULL error.");
                                list_free(list);
                                ret = RET_ERROR;
                            } else if(tmp == RET_ERROR) {
                                list_free(list);
                                ret = RET_ERROR;
                            } else {
                                for(int i = 0; tmp[i] != NULL; i++) {
                                    list_append(list, tmp[i]);
                                    object_dereference(tmp[i]);
                                }
                                _free(tmp);
                            }
                        }
                    }

                    if(vals_loc != RET_ERROR && vals_loc != NULL) {
                        _free(vals_loc);
                    }
                    if(vals_in != RET_ERROR && vals_in != NULL) {
                        for(int i = 0; vals_in[i] != NULL; i++)
                            object_dereference(vals_in[i]);
                        _free(vals_in);
                    }

                    if(ret != RET_ERROR) {
                        list_append(list, NULL);
                        ret = list->data;
                        _free(list);
                    }
                } break;
                case OPERATION_TYPE_IMPORT: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Import NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        for (int i = 0; ret != RET_ERROR && vals[i] != NULL; i++) {
                            if(vals[i]->type != OBJECT_TYPE_STRING) {
                                error("Runtime error: Import type error.");
                                ret = RET_ERROR;
                            } else {
                                FILE* file = fopen(string_get_cstr(vals[i]->data.string), "r");
                                if(file == NULL) {
                                    error("Runtime error: Import file error.");
                                    ret = RET_ERROR;
                                } else {
                                    old_error_handler = get_error_handler();
                                    set_error_handler(import_error_handler);

                                    fseek(file, 0, SEEK_END);
                                    size_t file_size = ftell(file);
                                    fseek(file, 0, SEEK_SET);

                                    char* buffer = (char*)_alloc(sizeof(char)*(file_size+1));
                                    buffer[file_size] = '\0';
                                    fread(buffer, 1, file_size, file);

                                    program_t* program = tokenize_and_parse_program(buffer);
                                    ret = operation_result(program, env);
                                    program_free(program);

                                    _free(buffer);
                                    fclose(file);

                                    set_error_handler(old_error_handler);

                                    if(import_error_flag) {
                                        ret = RET_ERROR;
                                    }
                                }
                            }
                        }
                    }
                } break;
                case OPERATION_TYPE_FOPEN: {
                    object_t** data = operation_result(op->data.operations[0], env);

                    if(data == NULL) {
                        error("Runtime error: fopen NULL error.");
                        ret = RET_ERROR;
                    } else if(data == RET_ERROR)
                        ret = RET_ERROR;
                    else if(data[1] == NULL) {
                        error("Runtime error: fopen Too few arguments error.");
                        ret = RET_ERROR;
                    } else if(data[2] != NULL) {
                        error("Runtime error: fopen Too many arguments error.");
                        ret = RET_ERROR;
                    } else if(data[0]->type != OBJECT_TYPE_STRING || data[1]->type != OBJECT_TYPE_STRING) {
                        error("Runtime error: fopen Type error.");
                        ret = RET_ERROR;
                    } else {
                        int flags = O_SYNC;
                        if(data[1]->data.string->length == 1) {
                            if(data[1]->data.string->data[0] == 'r') {
                                flags |= O_RDONLY;
                            } else if(data[1]->data.string->data[0] == 'w') {
                                flags |= O_WRONLY | O_CREAT;
                            } else if(data[1]->data.string->data[0] == 'a') {
                                flags |= O_WRONLY | O_APPEND | O_CREAT;
                            }
                        } else if(data[1]->data.string->length == 2 && data[1]->data.string->data[1] == '+') {
                            if(data[1]->data.string->data[0] == 'r') {
                                flags |= O_RDWR;
                            } else if(data[1]->data.string->data[0] == 'w') {
                                flags |= O_RDWR | O_CREAT;
                            } else if(data[1]->data.string->data[0] == 'a') {
                                flags |= O_RDWR | O_APPEND | O_CREAT;
                            }
                        }
                        ret = (object_t**)_alloc(sizeof(object_t*)*2);
                        int file = open(data[0]->data.string->data, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                        ret[0] = object_create_number((number_t)file);
                        object_reference(ret[0]);
                        ret[1] = NULL;
                    }
                    if(data != RET_ERROR && data != NULL) {
                        for(int i = 0; data[i] != NULL; i++)
                            object_dereference(data[i]);
                        _free(data);
                    }
                } break;
                case OPERATION_TYPE_FCLOSE: {
                    object_t** data = operation_result(op->data.operations[0], env);

                    if(data == NULL) {
                        error("Runtime error: fclose NULL error.");
                        ret = RET_ERROR;
                    } else if(data == RET_ERROR)
                        ret = RET_ERROR;
                    else if(data[1] != NULL) {
                        error("Runtime error: fclose Too many arguments error.");
                        ret = RET_ERROR;
                    } else if(data[0]->type != OBJECT_TYPE_NUMBER) {
                        error("Runtime error: fclose Type error.");
                        ret = RET_ERROR;
                    } else if(data[0]->data.number != (int)(data[0]->data.number)) {
                        error("Runtime error: fclose Integer error.");
                        ret = RET_ERROR;
                    } else {
                        int file = (int)(data[0]->data.number);
                        close(file);
                    }
                    if(data != RET_ERROR && data != NULL) {
                        for(int i = 0; data[i] != NULL; i++)
                            object_dereference(data[i]);
                        _free(data);
                    }
                } break;
                case OPERATION_TYPE_FREAD: {
                    object_t** data = operation_result(op->data.operations[0], env);

                    if(data == NULL) {
                        error("Runtime error: fread NULL error.");
                        ret = RET_ERROR;
                    } else if(data == RET_ERROR)
                        ret = RET_ERROR;
                    else if(data[1] != NULL) {
                        error("Runtime error: fread Too many arguments error.");
                        ret = RET_ERROR;
                    } else if(data[0]->type != OBJECT_TYPE_NUMBER) {
                        error("Runtime error: fread Type error.");
                        ret = RET_ERROR;
                    } else if(data[0]->data.number != (int)(data[0]->data.number)) {
                        error("Runtime error: fread Integer error.");
                        ret = RET_ERROR;
                    } else {
                        int file = (int)(data[0]->data.number);
                        size_t cap = 512;
                        size_t size = 0;
                        char* buffer = malloc(cap);
                        while(1) {
                            size_t ret = read(file, buffer + size, 1);
                            if(ret <= 0 || buffer[size] == '\n') {
                                break;
                            }
                            size++;
                            if(size == cap) {
                                cap *= 2;
                                buffer = realloc(buffer, cap);
                            }
                        }
                        ret = (object_t**)_alloc(sizeof(object_t*) * 2);
                        ret[0] = object_create_string(string_create_full(buffer, size));
                        object_reference(ret[0]);
                        ret[1] = NULL;
                        free(buffer);
                    }
                    if(data != RET_ERROR && data != NULL) {
                        for(int i = 0; data[i] != NULL; i++)
                            object_dereference(data[i]);
                        _free(data);
                    }
                } break;
                case OPERATION_TYPE_FWRITE: {
                    object_t** data = operation_result(op->data.operations[0], env);

                    if(data == NULL) {
                        error("Runtime error: fwrite NULL error.");
                        ret = RET_ERROR;
                    } else if(data == RET_ERROR)
                        ret = RET_ERROR;
                    else if(data[0]->type != OBJECT_TYPE_NUMBER) {
                        error("Runtime error: fwrite Type error.");
                        ret = RET_ERROR;
                    } else if(data[0]->data.number != (int)(data[0]->data.number)) {
                        error("Runtime error: fwrite Integer error.");
                        ret = RET_ERROR;
                    } else {
                        int file = (int)(data[0]->data.number);
                        for(int i = 1; data[i] != NULL; i++) {
                            string_t* str = object_to_string(data[i]);
                            write(file, str->data, str->length);
                            string_free(str);
                        }
                    }
                    if(data != RET_ERROR && data != NULL) {
                        for(int i = 0; data[i] != NULL; i++)
                            object_dereference(data[i]);
                        _free(data);
                    }
                } break;
            }
        }

    return ret;
}

object_t*** operation_var(operation_t* op, environment_t* env) {
    object_t*** ret = NULL;

    if(check_for_stackoverflow()) {
        error("Runtime error: Stack overflow.");
        ret = RET_ERROR;
    } else
        if(op != NULL) {
            switch(op->type) {
                case OPERATION_TYPE_NOOP:
                case OPERATION_TYPE_NOOP_BRAC:
                case OPERATION_TYPE_NOOP_EMP_REC:
                case OPERATION_TYPE_NOOP_O_LIST_DEADEND:
                case OPERATION_TYPE_NOOP_PROC_DEADEND:
                case OPERATION_TYPE_NOOP_PLUS:
                case OPERATION_TYPE_NOOP_EMP_CUR: break;
                case OPERATION_TYPE_NONE: break;
                case OPERATION_TYPE_NUM: break;
                case OPERATION_TYPE_STR: break;
                case OPERATION_TYPE_VAR:
                    ret = (object_t***)_alloc(sizeof(object_t**)*2);
                    environment_make(env, op->data.str);
                    ret[0] = environment_get_var(env, op->data.str);
                    ret[1] = NULL;
                    if((*(ret[0]))->type == OBJECT_TYPE_MACRO) {
                        object_t*** tmp = macro_var((*(ret[0]))->data.mac, env);
                        if(tmp == RET_ERROR) {
                            _free(ret);
                            ret = RET_ERROR;
                        } else {
                            _free(ret);
                            ret = tmp;
                        }
                    }
                    break;
                case OPERATION_TYPE_BOOL: break;
                case OPERATION_TYPE_PAIR: break;
                case OPERATION_TYPE_FUNCTION: break;
                case OPERATION_TYPE_MACRO: break;
                case OPERATION_TYPE_ASSIGN: {
                    object_t**    assign_val = operation_result(op->data.operations[1], env);
                    object_t*** assign_loc = operation_var(op->data.operations[0], env);

                    if(assign_loc == NULL || assign_val == NULL) {
                        error("Runtime error: Assignment NULL error.");
                        ret = RET_ERROR;
                    } else if(assign_loc == RET_ERROR || assign_val == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        for(int i = 0; assign_loc[i] != NULL && assign_val[i] != NULL; i++) {
                            if(assign_loc[i] == OBJECT_LIST_OPENED) {
                                object_t** obj = assign_loc[i+1];
                                object_dereference(*obj);

                                size_t length_left = 0;
                                while(assign_val[i + length_left] != NULL) length_left++;

                                list_t* list = list_create_null(length_left);
                                for(int j = 0; j < length_left; j++) {
                                    list->data[j] = assign_val[i+j];
                                    object_reference(list->data[j]);
                                }
                                *obj = object_create_list(list);
                                object_reference(*obj);
                                break;
                            } else {
                                object_dereference(*(assign_loc[i]));
                                *(assign_loc[i]) = assign_val[i];
                                object_reference(*(assign_loc[i]));
                            }
                        }
                    }

                    if(ret != RET_ERROR) {
                        ret = assign_loc;
                    }
                    else if(assign_loc != RET_ERROR && assign_loc != NULL) {
                        _free(assign_loc);
                    }
                    if(assign_val != RET_ERROR && assign_val != NULL) {
                        for(int i = 0; assign_val[i] != NULL; i++)
                            object_dereference(assign_val[i]);
                        _free(assign_val);
                    }
                } break;
                case OPERATION_TYPE_PROC:
                case OPERATION_TYPE_PROC_IMP: {
                    int i;
                    for(i = 0; ret != RET_ERROR && op->data.operations[i+1] != NULL; i++)
                        if(operation_exec(op->data.operations[i], env) == RET_ERROR)
                            ret = RET_ERROR;
                    if(ret != RET_ERROR)
                        ret = operation_var(op->data.operations[i], env);
                } break;
                case OPERATION_TYPE_STRUCT: break;
                case OPERATION_TYPE_O_LIST: {
                    size_t num_op = 0;
                    while(op->data.operations[num_op] != NULL) num_op++;

                    object_t**** vals = (object_t****)_alloc(sizeof(object_t***)*num_op);

                    size_t num_ret = 0;
                    for(int i = 0; ret != RET_ERROR && i < num_op; i++) {
                        vals[i] = operation_var(op->data.operations[i], env);
                        if(vals[i] == NULL) {
                            error("Runtime error: Open list NULL error.");
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else if(vals[i] == RET_ERROR) {
                            ret = RET_ERROR;
                            for(++i; i < num_op; i++)
                                vals[i] = NULL;
                        } else
                            for(int j = 0; vals[i][j] != NULL; j++)
                                num_ret++;
                    }
                    if(ret != RET_ERROR) {
                        ret = (object_t***)_alloc(sizeof(object_t**)*(num_ret+1));
                        num_ret = 0;
                        for(int i = 0; i < num_op; i++) {
                            for(int j = 0; vals[i][j] != NULL; j++)
                                ret[num_ret++] = vals[i][j];
                        }
                        ret[num_ret] = NULL;
                    }

                    for(int i = 0; i < num_op; i++)
                        if(ret != RET_ERROR) {
                            _free(vals[i]);
                        } else {
                            if(vals[i] != RET_ERROR && vals[i] != NULL)
                                _free(vals[i]);
                        }
                    _free(vals);
                } break;
                case OPERATION_TYPE_LIST: break;
                case OPERATION_TYPE_INDEX: {
                    object_t** data = operation_result(op->data.operations[0], env);
                    object_t** index = operation_result(op->data.operations[1], env);

                    if(data == NULL || index == NULL) {
                        error("Runtime error: Indexing NULL error.");
                        ret = RET_ERROR;
                    } else if(data == RET_ERROR || index == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_data = 0;
                        while(data[num_data] != NULL) num_data++;
                        size_t num_index = 0;
                        while(index[num_index] != NULL) num_index++;
                        ret = (object_t***)_alloc(sizeof(object_t**)*(num_data*num_index+1));

                        for(int i = 0; ret != RET_ERROR && i < num_data; i++)
                            for(int j = 0; ret != RET_ERROR && j < num_index; j++) {
                                if(data[i]->type == OBJECT_TYPE_LIST) {
                                    if(index[j]->type == OBJECT_TYPE_NUMBER) {

                                        pos_t ind = (int)round(index[j]->data.number);
                                        if(ind < 0)
                                            ind += data[i]->data.list->size;
                                        object_t** obj = list_get_loc(data[i]->data.list, ind);
                                        if(obj == NULL) {
                                            error("Runtime error: Indexing list out-of-range error.");
                                            _free(ret);
                                            ret = RET_ERROR;
                                        } else
                                            ret[i*num_index+j] = obj;

                                    } else {
                                        error("Runtime error: Indexing type error.");
                                        _free(ret);
                                        ret = RET_ERROR;
                                    }
                                } else if (data[i]->type == OBJECT_TYPE_DICTIONARY) {
                                    ret[i*num_index+j] = dictionary_get_loc(data[i]->data.dic, index[j]);
                                } else {
                                    error("Runtime error: Indexing type error.");
                                    _free(ret);
                                    ret = RET_ERROR;
                                }
                            }
                        if(ret != RET_ERROR)
                            ret[num_data*num_index] = NULL;
                    }

                    if(data != RET_ERROR && data != NULL) {
                        for(int i = 0; data[i] != NULL; i++)
                            object_dereference(data[i]);
                        _free(data);
                    }
                    if(index != RET_ERROR && index != NULL) {
                        for(int i = 0; index[i] != NULL; i++)
                            object_dereference(index[i]);
                        _free(index);
                    }
                } break;
                case OPERATION_TYPE_FOPEN: break;
                case OPERATION_TYPE_FCLOSE: break;
                case OPERATION_TYPE_FREAD: break;
                case OPERATION_TYPE_FWRITE: break;
                case OPERATION_TYPE_EXEC: break;
                case OPERATION_TYPE_TO_NUM: break;
                case OPERATION_TYPE_TO_BOOL: break;
                case OPERATION_TYPE_TO_ASCII: break;
                case OPERATION_TYPE_TO_STR: break;
                case OPERATION_TYPE_READ: break;
                case OPERATION_TYPE_WRITE: break;
                case OPERATION_TYPE_ADD: break;
                case OPERATION_TYPE_SUB: break;
                case OPERATION_TYPE_MUL: break;
                case OPERATION_TYPE_DIV: break;
                case OPERATION_TYPE_MOD: break;
                case OPERATION_TYPE_NEG: break;
                case OPERATION_TYPE_POW: break;
                case OPERATION_TYPE_AND: break;
                case OPERATION_TYPE_OR: break;
                case OPERATION_TYPE_XOR: break;
                case OPERATION_TYPE_NOT: break;
                case OPERATION_TYPE_SQRT: break;
                case OPERATION_TYPE_CBRT: break;
                case OPERATION_TYPE_SIN: break;
                case OPERATION_TYPE_COS: break;
                case OPERATION_TYPE_TAN: break;
                case OPERATION_TYPE_ASIN: break;
                case OPERATION_TYPE_ACOS: break;
                case OPERATION_TYPE_ATAN: break;
                case OPERATION_TYPE_SINH: break;
                case OPERATION_TYPE_COSH: break;
                case OPERATION_TYPE_TANH: break;
                case OPERATION_TYPE_ASINH: break;
                case OPERATION_TYPE_ACOSH: break;
                case OPERATION_TYPE_ATANH: break;
                case OPERATION_TYPE_TRUNC: break;
                case OPERATION_TYPE_FLOOR: break;
                case OPERATION_TYPE_CEIL: break;
                case OPERATION_TYPE_ROUND: break;
                case OPERATION_TYPE_RAND: break;
                case OPERATION_TYPE_LEN: break;
                case OPERATION_TYPE_EQU: break;
                case OPERATION_TYPE_GEQ: break;
                case OPERATION_TYPE_LEQ: break;
                case OPERATION_TYPE_GTR: break;
                case OPERATION_TYPE_LES: break;
                case OPERATION_TYPE_DIC: break;
                case OPERATION_TYPE_FIND: break;
                case OPERATION_TYPE_SPLIT: break;
                case OPERATION_TYPE_ABS: break;
                case OPERATION_TYPE_SCOPE:
                    environment_add_scope(env);
                    ret = operation_var(op->data.operations[0], env);
                    environment_remove_scope(env);
                break;
                case OPERATION_TYPE_IF: {
                    object_t** cond = operation_result(op->data.operations[0], env);

                    if(cond == NULL) {
                        error("Runtime error: If NULL error.");
                        ret = RET_ERROR;
                    } else if(cond == RET_ERROR) {
                        ret = RET_ERROR;
                    } else if(cond[1] != NULL) {
                        error("Runtime error: If non-scalar condition error.");
                        ret = RET_ERROR;
                    } else {
                        if(is_true(cond[0]))
                            ret = operation_var(op->data.operations[1], env);
                    }

                    if(cond != RET_ERROR && cond != NULL) {
                        for(int i = 0; cond[i] != NULL; i++)
                            object_dereference(cond[i]);
                        _free(cond);
                    }
                } break;
                case OPERATION_TYPE_IFELSE: {
                    object_t** cond = operation_result(op->data.operations[0], env);

                    if(cond == NULL) {
                        error("Runtime error: If-else NULL error.");
                        ret = RET_ERROR;
                    } else if(cond == RET_ERROR) {
                        ret = RET_ERROR;
                    } else if(cond[1] != NULL) {
                        error("Runtime error: If-else non-scalar condition error.");
                        ret = RET_ERROR;
                    } else {
                        if(is_true(cond[0]))
                            ret = operation_var(op->data.operations[1], env);
                        else
                            ret = operation_var(op->data.operations[2], env);
                    }

                    if(cond != RET_ERROR && cond != NULL) {
                        for(int i = 0; cond[i] != NULL; i++)
                            object_dereference(cond[i]);
                        _free(cond);
                    }
                }break;
                case OPERATION_TYPE_WHILE: {
                    object_t** cond = operation_result(op->data.operations[0], env);

                    if(cond == NULL) {
                        error("Runtime error: While NULL error.");
                        ret = RET_ERROR;
                    } else if(cond == RET_ERROR) {
                        ret = RET_ERROR;
                    } else if(cond[1] != NULL) {
                        error("Runtime error: While non-scalar condition error.");
                        ret = RET_ERROR;
                    } else {
                        ret = (object_t***)_alloc(sizeof(object_t**));
                        ret[0] = NULL;
                        size_t num_ret = 0;

                        while(ret != RET_ERROR && is_true(cond[0]))
                        {
                            object_t*** tmp = operation_var(op->data.operations[1], env);
                            if(tmp == NULL) {
                                error("Runtime error: While NULL error.");
                                _free(ret);
                                ret = RET_ERROR;
                            } else if(tmp == RET_ERROR) {
                                _free(ret);
                                ret = RET_ERROR;
                            } else {
                                for(int i = 0; tmp[i] != NULL; i++) {
                                    object_t*** tmp_ret = (object_t***)_alloc(sizeof(object_t**)*(num_ret+2));
                                    for(int j = 0; j < num_ret; j++)
                                        tmp_ret[j] = ret[j];
                                    tmp_ret[num_ret] = tmp[i];
                                    num_ret++;
                                    tmp_ret[num_ret] = NULL;
                                    _free(ret);
                                    ret = tmp_ret;
                                }
                                _free(tmp);

                                object_dereference(cond[1]);
                                _free(cond);
                                cond = operation_result(op->data.operations[0], env);
                                if(cond == NULL) {
                                    error("Runtime error: While NULL error.");
                                    _free(ret);
                                    ret = RET_ERROR;
                                } else if(cond == RET_ERROR) {
                                    _free(ret);
                                    ret = RET_ERROR;
                                } else if(cond[1] != NULL) {
                                    error("Runtime error: While non-scalar condition error.");
                                    _free(ret);
                                    ret = RET_ERROR;
                                }
                            }
                        }
                    }

                    if(cond != RET_ERROR && cond != NULL) {
                        for(int i = 0; cond[i] != NULL; i++)
                            object_dereference(cond[i]);
                        _free(cond);
                    }
                } break;
                case OPERATION_TYPE_IN_STRUCT: {
                    object_t** stc = operation_result(op->data.operations[0], env);

                    if(stc == NULL) {
                        error("Runtime error: Struct NULL error.");
                        ret = RET_ERROR;
                    } else if(stc == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        size_t num_stc = 0;
                        while(stc[num_stc] != NULL) num_stc++;

                        object_t**** vals = (object_t****)_alloc(sizeof(object_t***)*num_stc);

                        size_t num_ret = 0;
                        for(int i = 0; ret != RET_ERROR && i < num_stc; i++) {
                            if(stc[i]->type != OBJECT_TYPE_STRUCT) {
                                error("Runtime error: Struct type error.");
                                ret = RET_ERROR;
                                for(; i < num_stc; i++)
                                    vals[i] = NULL;
                            } else {
                                vals[i] = struct_var(stc[i]->data.stc, op->data.operations[1]);
                                if(vals[i] == NULL) {
                                    error("Runtime error: Struct NULL error");
                                    ret = RET_ERROR;
                                    for(++i; i < num_stc; i++)
                                        vals[i] = NULL;
                                } else if(vals[i] == RET_ERROR) {
                                    ret = RET_ERROR;
                                    for(++i; i < num_stc; i++)
                                        vals[i] = NULL;
                                } else
                                    for(int j = 0; vals[i][j] != NULL; j++)
                                        num_ret++;
                            }
                        }
                        if(ret != RET_ERROR) {
                            ret = (object_t***)_alloc(sizeof(object_t**)*(num_ret+1));
                            num_ret = 0;
                            for(int i = 0; i < num_stc; i++) {
                                for(int j = 0; vals[i][j] != NULL; j++) {
                                    ret[num_ret] = vals[j][i];
                                    num_ret++;
                                }
                            }
                            ret[num_ret] = NULL;
                        }

                        for(int i = 0; i < num_stc; i++)
                            if(vals[i] != RET_ERROR && vals[i] != NULL)
                                _free(vals[i]);
                        _free(vals);
                        for(int i = 0; i < num_stc; i++)
                            object_dereference(stc[i]);
                        _free(stc);
                    }
                } break;
                case OPERATION_TYPE_LOCAL: {
                    size_t prev_limit = env->local_mode_limit;
                    environment_set_local_mode(env, env->count-1);
                    ret = operation_var(op->data.operations[0], env);
                    environment_set_local_mode(env, prev_limit);
                } break;
                case OPERATION_TYPE_GLOBAL: {
                    size_t prev_limit = env->local_mode_limit;
                    environment_set_local_mode(env, 0);
                    ret = operation_var(op->data.operations[0], env);
                    environment_set_local_mode(env, prev_limit);
                } break;
                case OPERATION_TYPE_COPY: break;
                case OPERATION_TYPE_FOR: {
                    if(operation_exec(op->data.operations[0], env) == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        object_t** cond = operation_result(op->data.operations[1], env);

                        if(cond == NULL) {
                            error("Runtime error: For NULL error.");
                            ret = RET_ERROR;
                        } else if(cond == RET_ERROR) {
                            ret = RET_ERROR;
                        } else if(cond[1] != NULL) {
                            error("Runtime error: For non-scalar condition error.");
                            ret = RET_ERROR;
                        } else {
                            ret = (object_t***)_alloc(sizeof(object_t**));
                            ret[0] = NULL;
                            size_t num_ret = 0;

                            while(ret != RET_ERROR && is_true(cond[0]))
                            {
                                object_t*** tmp = operation_var(op->data.operations[3], env);
                                if(tmp == NULL) {
                                    error("Runtime error: For NULL error.");
                                    _free(ret);
                                    ret = RET_ERROR;
                                } else if(tmp == RET_ERROR) {
                                    _free(ret);
                                    ret = RET_ERROR;
                                } else if(operation_exec(op->data.operations[2], env) == RET_ERROR) {
                                    ret = RET_ERROR;
                                } else {
                                    for(int i = 0; tmp[i] != NULL; i++) {
                                        object_t*** tmp_ret = (object_t***)_alloc(sizeof(object_t**)*(num_ret+2));
                                        for(int j = 0; j < num_ret; j++)
                                            tmp_ret[j] = ret[j];
                                        tmp_ret[num_ret] = tmp[i];
                                        num_ret++;
                                        tmp_ret[num_ret] = NULL;
                                        _free(ret);
                                        ret = tmp_ret;
                                    }
                                    _free(tmp);

                                    object_dereference(cond[1]);
                                    _free(cond);
                                    cond = operation_result(op->data.operations[1], env);
                                    if(cond == NULL) {
                                        error("Runtime error: For NULL error.");
                                        _free(ret);
                                        ret = RET_ERROR;
                                    } else if(cond == RET_ERROR) {
                                        _free(ret);
                                        ret = RET_ERROR;
                                    } else if(cond[1] != NULL) {
                                        error("Runtime error: For non-scalar condition error.");
                                        _free(ret);
                                        ret = RET_ERROR;
                                    }
                                }
                            }
                        }

                        if(cond != RET_ERROR && cond != NULL) {
                            for(int i = 0; cond[i] != NULL; i++)
                                object_dereference(cond[i]);
                            _free(cond);
                        }
                    }
                } break;
                case OPERATION_TYPE_LIST_OPEN: {
                    object_t*** vals = operation_var(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: List-opening NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        if(vals[0] == OBJECT_LIST_OPENED) {
                            error("Runtime error: List-open type error.");
                            ret = RET_ERROR;
                        } else {
                            ret = (object_t***)_alloc(sizeof(object_t**)*3);
                            ret[0] = OBJECT_LIST_OPENED;
                            ret[1] = vals[0];
                            ret[2] = NULL;

                            _free(vals);
                        }
                    }
                } break;
                case OPERATION_TYPE_FOR_IN: {
                    object_t*** vals_loc = operation_var(op->data.operations[0], env);
                    object_t** vals_in = operation_result(op->data.operations[1], env);

                    if(vals_loc == NULL || vals_in == NULL) {
                        error("Runtime error: For-in NULL error.");
                        ret = RET_ERROR;
                    } else if(vals_loc == RET_ERROR || vals_in == RET_ERROR) {
                        ret = RET_ERROR;
                    } else if(vals_in[0] != NULL){
                        ret = (object_t***)_alloc(sizeof(object_t**));
                        ret[0] = NULL;
                        size_t num_ret = 0;

                        int pos_in = 0;
                        while(ret != RET_ERROR && vals_in[pos_in] != NULL) {
                            // Assign values
                            for(int i = 0; vals_loc[i] != NULL && vals_in[pos_in] != NULL; i++) {
                                if(vals_loc[i] == OBJECT_LIST_OPENED) {
                                    object_t** obj = vals_loc[i+1];
                                    object_dereference(*obj);

                                    size_t length_left = 0;
                                    while(vals_in[pos_in + length_left] != NULL) length_left++;

                                    list_t* list = list_create_null(length_left);
                                    for(int j = 0; j < length_left; j++) {
                                        list->data[j] = vals_in[i+j];
                                        object_reference(list->data[j]);
                                    }
                                    *obj = object_create_list(list);
                                    object_reference(*obj);
                                    pos_in += length_left;
                                } else {
                                    object_dereference(*(vals_loc[i]));
                                    *(vals_loc[i]) = vals_in[pos_in];
                                    object_reference(*(vals_loc[i]));
                                    pos_in++;
                                }
                            }

                            object_t*** tmp = operation_var(op->data.operations[2], env);
                            if(tmp == NULL) {
                                error("Runtime error: For NULL error.");
                                _free(ret);
                                ret = RET_ERROR;
                            } else if(tmp == RET_ERROR) {
                                _free(ret);
                                ret = RET_ERROR;
                            } else {
                                for(int i = 0; tmp[i] != NULL; i++) {
                                    object_t*** tmp_ret = (object_t***)_alloc(sizeof(object_t**)*(num_ret+2));
                                    for(int j = 0; j < num_ret; j++)
                                        tmp_ret[j] = ret[j];
                                    tmp_ret[num_ret] = tmp[i];
                                    num_ret++;
                                    tmp_ret[num_ret] = NULL;
                                    _free(ret);
                                    ret = tmp_ret;
                                }
                                _free(tmp);
                            }
                        }
                    }

                    if(vals_loc != RET_ERROR && vals_loc != NULL) {
                        _free(vals_loc);
                    }
                    if(vals_in != RET_ERROR && vals_in != NULL) {
                        for(int i = 0; vals_in[i] != NULL; i++)
                            object_dereference(vals_in[i]);
                        _free(vals_in);
                    }
                } break;
                case OPERATION_TYPE_IMPORT: {
                    object_t** vals = operation_result(op->data.operations[0], env);

                    if(vals == NULL) {
                        error("Runtime error: Import NULL error.");
                        ret = RET_ERROR;
                    } else if(vals == RET_ERROR) {
                        ret = RET_ERROR;
                    } else {
                        for (int i = 0; ret != RET_ERROR && vals[i] != NULL; i++) {
                            if(vals[i]->type != OBJECT_TYPE_STRING) {
                                error("Runtime error: Import type error.");
                                ret = RET_ERROR;
                            } else {
                                FILE* file = fopen(string_get_cstr(vals[i]->data.string), "r");
                                if(file == NULL) {
                                    error("Runtime error: Import file error.");
                                    ret = RET_ERROR;
                                } else {
                                    old_error_handler = get_error_handler();
                                    set_error_handler(import_error_handler);

                                    fseek(file, 0, SEEK_END);
                                    size_t file_size = ftell(file);
                                    fseek(file, 0, SEEK_SET);

                                    char* buffer = (char*)_alloc(sizeof(char)*(file_size+1));
                                    buffer[file_size] = '\0';
                                    fread(buffer, 1, file_size, file);

                                    program_t* program = tokenize_and_parse_program(buffer);
                                    ret = operation_var(program, env);
                                    program_free(program);

                                    _free(buffer);
                                    fclose(file);

                                    set_error_handler(old_error_handler);

                                    if(import_error_flag) {
                                        ret = RET_ERROR;
                                    }
                                }
                            }
                        }
                    }
                } break;
            }
        }

    return ret;
}

void operation_free(operation_t* op) {
    if(op != NULL) {
        switch(op->type) {
            case OPERATION_TYPE_NOOP:
            case OPERATION_TYPE_NOOP_BRAC:
            case OPERATION_TYPE_NOOP_EMP_REC:
            case OPERATION_TYPE_NOOP_O_LIST_DEADEND:
            case OPERATION_TYPE_NOOP_PROC_DEADEND:
            case OPERATION_TYPE_NOOP_PLUS:
            case OPERATION_TYPE_NOOP_EMP_CUR: break;
            case OPERATION_TYPE_NONE: break;
            case OPERATION_TYPE_NUM: break;
            case OPERATION_TYPE_STR:
            case OPERATION_TYPE_VAR: string_free(op->data.str); break;
            case OPERATION_TYPE_BOOL: break;
            case OPERATION_TYPE_PAIR:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_FUNCTION:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_MACRO:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_ASSIGN:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_PROC:
            case OPERATION_TYPE_PROC_IMP:
                for(int i = 0; op->data.operations[i] != NULL; i++)
                    operation_free(op->data.operations[i]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_STRUCT:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_O_LIST:
                for(int i = 0; op->data.operations[i] != NULL; i++)
                    operation_free(op->data.operations[i]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_LIST:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_INDEX:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_EXEC:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_TO_NUM:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_TO_BOOL:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_TO_ASCII:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_TO_STR:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_READ: break;
            case OPERATION_TYPE_WRITE:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_ADD:
                for(int i = 0; op->data.operations[i] != NULL; i++)
                    operation_free(op->data.operations[i]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_SUB:
                for(int i = 0; op->data.operations[i] != NULL; i++)
                    operation_free(op->data.operations[i]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_MUL:
                for(int i = 0; op->data.operations[i] != NULL; i++)
                    operation_free(op->data.operations[i]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_DIV:
                for(int i = 0; op->data.operations[i] != NULL; i++)
                    operation_free(op->data.operations[i]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_MOD:
                for(int i = 0; op->data.operations[i] != NULL; i++)
                    operation_free(op->data.operations[i]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_NEG:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_POW:
                for(int i = 0; op->data.operations[i] != NULL; i++)
                    operation_free(op->data.operations[i]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_AND:
                for(int i = 0; op->data.operations[i] != NULL; i++)
                    operation_free(op->data.operations[i]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_OR:
                for(int i = 0; op->data.operations[i] != NULL; i++)
                    operation_free(op->data.operations[i]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_XOR:
                for(int i = 0; op->data.operations[i] != NULL; i++)
                    operation_free(op->data.operations[i]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_NOT:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_SQRT:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_CBRT:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_SIN:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_COS:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_TAN:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_ASIN:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_ACOS:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_ATAN:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_SINH:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_COSH:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_TANH:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_ASINH:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_ACOSH:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_ATANH:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_TRUNC:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_FLOOR:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_CEIL:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_ROUND:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_RAND: break;
            case OPERATION_TYPE_LEN:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_EQU:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_GEQ:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_LEQ:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_GTR:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_LES:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_FIND:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_SPLIT:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_ABS:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_SCOPE:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_IF:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_IFELSE:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                operation_free(op->data.operations[2]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_WHILE:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_IN_STRUCT:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_DIC:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_LOCAL:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_GLOBAL:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_COPY:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_FOR:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                operation_free(op->data.operations[2]);
                operation_free(op->data.operations[3]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_LIST_OPEN:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_FOR_IN:
                operation_free(op->data.operations[0]);
                operation_free(op->data.operations[1]);
                operation_free(op->data.operations[2]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_IMPORT:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_FOPEN:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_FCLOSE:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_FREAD:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
            case OPERATION_TYPE_FWRITE:
                operation_free(op->data.operations[0]);
                _free(op->data.operations);
            break;
        }
        _free(op);
    }
}

id_t operation_id(operation_t* op) {
    id_t ret = 0;

    if(op != NULL) {
        switch(op->type) {
            case OPERATION_TYPE_NOOP:
            case OPERATION_TYPE_NOOP_BRAC:
            case OPERATION_TYPE_NOOP_EMP_REC:
            case OPERATION_TYPE_NOOP_O_LIST_DEADEND:
            case OPERATION_TYPE_NOOP_PROC_DEADEND:
            case OPERATION_TYPE_NOOP_PLUS:
            case OPERATION_TYPE_NOOP_EMP_CUR: break;
            case OPERATION_TYPE_NUM: break;
            case OPERATION_TYPE_NONE: break;
            case OPERATION_TYPE_STR: break;
            case OPERATION_TYPE_VAR: break;
            case OPERATION_TYPE_BOOL: break;
            case OPERATION_TYPE_PAIR: break;
            case OPERATION_TYPE_FUNCTION: break;
            case OPERATION_TYPE_MACRO: break;
            case OPERATION_TYPE_ASSIGN: break;
            case OPERATION_TYPE_PROC:
            case OPERATION_TYPE_PROC_IMP: break;
            case OPERATION_TYPE_STRUCT: break;
            case OPERATION_TYPE_O_LIST: break;
            case OPERATION_TYPE_LIST: break;
            case OPERATION_TYPE_INDEX: break;
            case OPERATION_TYPE_EXEC: break;
            case OPERATION_TYPE_TO_NUM: break;
            case OPERATION_TYPE_TO_BOOL: break;
            case OPERATION_TYPE_TO_ASCII: break;
            case OPERATION_TYPE_TO_STR: break;
            case OPERATION_TYPE_READ: break;
            case OPERATION_TYPE_WRITE: break;
            case OPERATION_TYPE_ADD: break;
            case OPERATION_TYPE_SUB: break;
            case OPERATION_TYPE_MUL: break;
            case OPERATION_TYPE_DIV: break;
            case OPERATION_TYPE_MOD: break;
            case OPERATION_TYPE_NEG: break;
            case OPERATION_TYPE_POW: break;
            case OPERATION_TYPE_AND: break;
            case OPERATION_TYPE_OR: break;
            case OPERATION_TYPE_XOR: break;
            case OPERATION_TYPE_NOT: break;
            case OPERATION_TYPE_SQRT: break;
            case OPERATION_TYPE_CBRT: break;
            case OPERATION_TYPE_SIN: break;
            case OPERATION_TYPE_COS: break;
            case OPERATION_TYPE_TAN: break;
            case OPERATION_TYPE_ASIN: break;
            case OPERATION_TYPE_ACOS: break;
            case OPERATION_TYPE_ATAN: break;
            case OPERATION_TYPE_SINH: break;
            case OPERATION_TYPE_COSH: break;
            case OPERATION_TYPE_TANH: break;
            case OPERATION_TYPE_ASINH: break;
            case OPERATION_TYPE_ACOSH: break;
            case OPERATION_TYPE_ATANH: break;
            case OPERATION_TYPE_TRUNC: break;
            case OPERATION_TYPE_FLOOR: break;
            case OPERATION_TYPE_CEIL: break;
            case OPERATION_TYPE_ROUND: break;
            case OPERATION_TYPE_RAND: break;
            case OPERATION_TYPE_LEN: break;
            case OPERATION_TYPE_EQU: break;
            case OPERATION_TYPE_GEQ: break;
            case OPERATION_TYPE_LEQ: break;
            case OPERATION_TYPE_GTR: break;
            case OPERATION_TYPE_LES: break;
            case OPERATION_TYPE_FIND: break;
            case OPERATION_TYPE_SPLIT: break;
            case OPERATION_TYPE_ABS: break;
            case OPERATION_TYPE_SCOPE: break;
            case OPERATION_TYPE_IF: break;
            case OPERATION_TYPE_IFELSE: break;
            case OPERATION_TYPE_WHILE: break;
            case OPERATION_TYPE_IN_STRUCT: break;
            case OPERATION_TYPE_DIC: break;
            case OPERATION_TYPE_LOCAL: break;
            case OPERATION_TYPE_GLOBAL: break;
            case OPERATION_TYPE_COPY: break;
            case OPERATION_TYPE_FOR: break;
            case OPERATION_TYPE_LIST_OPEN: break;
            case OPERATION_TYPE_FOR_IN: break;
            case OPERATION_TYPE_IMPORT: break;
            case OPERATION_TYPE_FOPEN: break;
            case OPERATION_TYPE_FREAD: break;
            case OPERATION_TYPE_FWRITE: break;
            case OPERATION_TYPE_FCLOSE: break;
        }
    }

    return ret;
}

bool_t operation_equ(operation_t* o1, operation_t* o2) {
    if(o1 == NULL && o2 == NULL)
        return true;
    else if(o1 == NULL || o2 == NULL)
        return false;
    else if(o1->type != o2->type)
        return false;

    bool_t ret = true;

    switch(o1->type) {
        case OPERATION_TYPE_NOOP:
        case OPERATION_TYPE_NOOP_BRAC:
        case OPERATION_TYPE_NOOP_EMP_REC:
        case OPERATION_TYPE_NOOP_O_LIST_DEADEND:
        case OPERATION_TYPE_NOOP_PLUS:
        case OPERATION_TYPE_NOOP_PROC_DEADEND:
        case OPERATION_TYPE_NOOP_EMP_CUR: ret = true; break;
        case OPERATION_TYPE_NONE: ret = true; break;
        case OPERATION_TYPE_NUM: ret = number_equ(o1->data.num, o2->data.num); break;
        case OPERATION_TYPE_STR:
        case OPERATION_TYPE_VAR: ret = string_equ(o1->data.str, o2->data.str); break;
        case OPERATION_TYPE_BOOL: ret = bool_equ(o1->data.boolean, o2->data.boolean); break;
        case OPERATION_TYPE_PAIR: ret = operation_equ(o1->data.operations[0], o2->data.operations[0]) && operation_equ(o1->data.operations[1], o2->data.operations[1]); break;
        case OPERATION_TYPE_FUNCTION: break;
        case OPERATION_TYPE_MACRO: break;
        case OPERATION_TYPE_ASSIGN: break;
        case OPERATION_TYPE_PROC:
        case OPERATION_TYPE_PROC_IMP:break;
        case OPERATION_TYPE_STRUCT: break;
        case OPERATION_TYPE_O_LIST: break;
        case OPERATION_TYPE_LIST: break;
        case OPERATION_TYPE_INDEX: break;
        case OPERATION_TYPE_EXEC: break;
        case OPERATION_TYPE_TO_NUM: break;
        case OPERATION_TYPE_TO_BOOL: break;
        case OPERATION_TYPE_TO_ASCII: break;
        case OPERATION_TYPE_TO_STR: break;
        case OPERATION_TYPE_READ: break;
        case OPERATION_TYPE_WRITE: break;
        case OPERATION_TYPE_ADD: break;
        case OPERATION_TYPE_SUB: break;
        case OPERATION_TYPE_MUL: break;
        case OPERATION_TYPE_DIV: break;
        case OPERATION_TYPE_MOD: break;
        case OPERATION_TYPE_NEG: break;
        case OPERATION_TYPE_POW: break;
        case OPERATION_TYPE_AND: break;
        case OPERATION_TYPE_OR: break;
        case OPERATION_TYPE_XOR: break;
        case OPERATION_TYPE_NOT: break;
        case OPERATION_TYPE_SQRT: break;
        case OPERATION_TYPE_CBRT: break;
        case OPERATION_TYPE_SIN: break;
        case OPERATION_TYPE_COS: break;
        case OPERATION_TYPE_TAN: break;
        case OPERATION_TYPE_ASIN: break;
        case OPERATION_TYPE_ACOS: break;
        case OPERATION_TYPE_ATAN: break;
        case OPERATION_TYPE_SINH: break;
        case OPERATION_TYPE_COSH: break;
        case OPERATION_TYPE_TANH: break;
        case OPERATION_TYPE_ASINH: break;
        case OPERATION_TYPE_ACOSH: break;
        case OPERATION_TYPE_ATANH: break;
        case OPERATION_TYPE_TRUNC: break;
        case OPERATION_TYPE_FLOOR: break;
        case OPERATION_TYPE_CEIL: break;
        case OPERATION_TYPE_ROUND: break;
        case OPERATION_TYPE_RAND: break;
        case OPERATION_TYPE_LEN: break;
        case OPERATION_TYPE_EQU: break;
        case OPERATION_TYPE_GEQ: break;
        case OPERATION_TYPE_LEQ: break;
        case OPERATION_TYPE_GTR: break;
        case OPERATION_TYPE_LES: break;
        case OPERATION_TYPE_FIND: break;
        case OPERATION_TYPE_SPLIT: break;
        case OPERATION_TYPE_ABS: break;
        case OPERATION_TYPE_SCOPE: break;
        case OPERATION_TYPE_IF: break;
        case OPERATION_TYPE_IFELSE: break;
        case OPERATION_TYPE_WHILE: break;
        case OPERATION_TYPE_DIC: break;
        case OPERATION_TYPE_IN_STRUCT: break;
        case OPERATION_TYPE_LOCAL: break;
        case OPERATION_TYPE_GLOBAL: break;
        case OPERATION_TYPE_COPY: break;
        case OPERATION_TYPE_FOR: break;
        case OPERATION_TYPE_LIST_OPEN: break;
        case OPERATION_TYPE_FOR_IN: break;
        case OPERATION_TYPE_IMPORT: break;
        case OPERATION_TYPE_FOPEN: break;
        case OPERATION_TYPE_FREAD: break;
        case OPERATION_TYPE_FWRITE: break;
        case OPERATION_TYPE_FCLOSE: break;
    }

    return ret;
}

operation_t* operation_copy(operation_t* op) {
    operation_t* ret = operation_create();
    ret->type = op->type;

    switch (op->type) {
        case OPERATION_TYPE_VAR:
        case OPERATION_TYPE_STR:
            ret->data.str = string_copy(op->data.str);
        break;
        case OPERATION_TYPE_NUM:
            ret->data.num = op->data.num;
        break;
        case OPERATION_TYPE_BOOL:
            ret->data.boolean = op->data.boolean;
        break;
        case OPERATION_TYPE_NOOP:
        case OPERATION_TYPE_NONE:
        case OPERATION_TYPE_READ:
        case OPERATION_TYPE_RAND:
        break;
        case OPERATION_TYPE_EXEC:
        case OPERATION_TYPE_INDEX:
        case OPERATION_TYPE_IN_STRUCT:
        case OPERATION_TYPE_PAIR:
        case OPERATION_TYPE_ASSIGN:
        case OPERATION_TYPE_EQU:
        case OPERATION_TYPE_GTR:
        case OPERATION_TYPE_LES:
        case OPERATION_TYPE_LEQ:
        case OPERATION_TYPE_GEQ:
        case OPERATION_TYPE_FIND:
        case OPERATION_TYPE_SPLIT:
        case OPERATION_TYPE_IF:
        case OPERATION_TYPE_WHILE:
        case OPERATION_TYPE_FUNCTION:
            ret->data.operations = (operation_t**)_alloc(sizeof(operation_t*)*2);
            ret->data.operations[0] = operation_copy(op->data.operations[0]);
            ret->data.operations[1] = operation_copy(op->data.operations[1]);
        break;
        case OPERATION_TYPE_IFELSE:
        case OPERATION_TYPE_FOR_IN:
            ret->data.operations = (operation_t**)_alloc(sizeof(operation_t*)*3);
            ret->data.operations[0] = operation_copy(op->data.operations[0]);
            ret->data.operations[1] = operation_copy(op->data.operations[1]);
            ret->data.operations[2] = operation_copy(op->data.operations[2]);
        break;
        case OPERATION_TYPE_FOPEN: break;
        case OPERATION_TYPE_FREAD: break;
        case OPERATION_TYPE_FWRITE: break;
        case OPERATION_TYPE_FCLOSE: break;
        case OPERATION_TYPE_MACRO:
        case OPERATION_TYPE_STRUCT:
        case OPERATION_TYPE_DIC:
        case OPERATION_TYPE_SIN:
        case OPERATION_TYPE_COS:
        case OPERATION_TYPE_TAN:
        case OPERATION_TYPE_ASIN:
        case OPERATION_TYPE_ACOS:
        case OPERATION_TYPE_ATAN:
        case OPERATION_TYPE_SINH:
        case OPERATION_TYPE_COSH:
        case OPERATION_TYPE_TANH:
        case OPERATION_TYPE_ASINH:
        case OPERATION_TYPE_ACOSH:
        case OPERATION_TYPE_ATANH:
        case OPERATION_TYPE_TRUNC:
        case OPERATION_TYPE_FLOOR:
        case OPERATION_TYPE_CEIL:
        case OPERATION_TYPE_ROUND:
        case OPERATION_TYPE_LEN:
        case OPERATION_TYPE_CBRT:
        case OPERATION_TYPE_SQRT:
        case OPERATION_TYPE_TO_STR:
        case OPERATION_TYPE_TO_NUM:
        case OPERATION_TYPE_TO_BOOL:
        case OPERATION_TYPE_TO_ASCII:
        case OPERATION_TYPE_WRITE:
        case OPERATION_TYPE_NEG:
        case OPERATION_TYPE_NOT:
        case OPERATION_TYPE_LOCAL:
        case OPERATION_TYPE_GLOBAL:
        case OPERATION_TYPE_COPY:
        case OPERATION_TYPE_IMPORT:
        case OPERATION_TYPE_LIST_OPEN:
        case OPERATION_TYPE_LIST:
        case OPERATION_TYPE_SCOPE:
        case OPERATION_TYPE_ABS:
        case OPERATION_TYPE_NOOP_PLUS:
        case OPERATION_TYPE_NOOP_BRAC:
        case OPERATION_TYPE_NOOP_EMP_CUR:
        case OPERATION_TYPE_NOOP_EMP_REC:
            ret->data.operations = (operation_t**)_alloc(sizeof(operation_t*));
            ret->data.operations[0] = operation_copy(op->data.operations[0]);
        break;
        case OPERATION_TYPE_FOR:
            ret->data.operations = (operation_t**)_alloc(sizeof(operation_t*)*4);
            ret->data.operations[0] = operation_copy(op->data.operations[0]);
            ret->data.operations[1] = operation_copy(op->data.operations[1]);
            ret->data.operations[2] = operation_copy(op->data.operations[2]);
            ret->data.operations[3] = operation_copy(op->data.operations[3]);
        break;
        case OPERATION_TYPE_PROC:
        case OPERATION_TYPE_PROC_IMP:
        case OPERATION_TYPE_O_LIST:
        case OPERATION_TYPE_ADD:
        case OPERATION_TYPE_SUB:
        case OPERATION_TYPE_MUL:
        case OPERATION_TYPE_DIV:
        case OPERATION_TYPE_MOD:
        case OPERATION_TYPE_POW:
        case OPERATION_TYPE_AND:
        case OPERATION_TYPE_OR:
        case OPERATION_TYPE_XOR: {
            int num_operations = 0;
            while(op->data.operations[num_operations] != NULL)
                num_operations++;
            ret->data.operations = (operation_t**)_alloc(sizeof(operation_t*)*(num_operations+1));
            for(int i = 0; i < num_operations; i++)
                ret->data.operations[i] = operation_copy(op->data.operations[i]);
            ret->data.operations[num_operations] = NULL;
        } break;
        case OPERATION_TYPE_NOOP_O_LIST_DEADEND:
        case OPERATION_TYPE_NOOP_PROC_DEADEND:
        break;
    }

    return ret;
}
