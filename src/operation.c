// Copyright (c) 2018 Roland Bernard

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "./operation.h"
#include "./object.h"
#include "./langallocator.h"
#include "./prime.h"
#include "./error.h"

#define TMP_STR_MAX 1<<12

operation_t* operation_create() {
	return (operation_t*)_alloc(sizeof(operation_t));
}

operation_t* operation_create_NOOP() {
	operation_t* ret = (operation_t*)_alloc(sizeof(operation_t));

	ret->type = OPERATION_TYPE_NOOP;

	return ret;
}

void operation_exec(operation_t* op, environment_t* env) {
	if(check_for_stackoverflow())
		error("Runtime error: Stack overflow.");
	
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
					macro_exec(var->data.mac, env);
			} break;
			case OPERATION_TYPE_BOOL: break;
			case OPERATION_TYPE_PAIR: 
				operation_exec(op->data.operations[0], env); 
				operation_exec(op->data.operations[1], env);
				break;
			case OPERATION_TYPE_FUNCTION: break;
			case OPERATION_TYPE_MACRO: break;
			case OPERATION_TYPE_ASSIGN: {
				object_t*** assign_loc = operation_var(op->data.operations[0], env);
				object_t**	assign_val = operation_result(op->data.operations[1], env);

				if(assign_loc == NULL || assign_val == NULL)
					error("Runtime error: Assignment NULL error.");
				else
					for(int i = 0; assign_loc[i] != NULL && assign_val[i] != NULL; i++) {
						object_dereference(*(assign_loc[i]));
						*(assign_loc[i]) = assign_val[i];
						object_reference(*(assign_loc[i]));
					}
					
				_free(assign_loc);
				for(int i = 0; assign_val[i] != NULL; i++)
					object_dereference(assign_val[i]);
				_free(assign_val);
			} break;
			case OPERATION_TYPE_PROC: 
				for(int i = 0; op->data.operations[i] != NULL; i++)
					operation_exec(op->data.operations[i], env);
				break;
			case OPERATION_TYPE_STRUCT: 
				operation_exec(op->data.operations[0], env); 
				break;
			case OPERATION_TYPE_O_LIST:
				for(int i = 0; op->data.operations[i] != NULL; i++)
					operation_exec(op->data.operations[i], env);
				break;
			case OPERATION_TYPE_LIST: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_INDEX: 
				operation_exec(op->data.operations[0], env); 
				operation_exec(op->data.operations[1], env);
				break;
			case OPERATION_TYPE_EXEC: {
				object_t** function = operation_result(op->data.operations[0], env);
				object_t** parameter = operation_result(op->data.operations[1], env);

				if(function == NULL)
					error("Runtime error: Function NULL error.");
				else
					for(int i = 0; function[i] != NULL; i++)
						if(function[i]->type != OBJECT_TYPE_FUNCTION)
							error("Runtime error: Function type error.");
						else
							function_exec(function[i]->data.func, parameter, env);

				for(int i = 0; function[i] != NULL; i++)
					object_dereference(function[i]);
				_free(function);
				for(int i = 0; parameter[i] != NULL; i++)
					object_dereference(parameter[i]);
				_free(parameter);
			} break;
			case OPERATION_TYPE_TO_NUM: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_TO_BOOL: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_TO_STR: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_READ: {
				char temp_str[TMP_STR_MAX];
				fgets(temp_str, TMP_STR_MAX, stdin); 
			} break;
			case OPERATION_TYPE_WRITE: {
				object_t** data = operation_result(op->data.operations[0], env);

				if(data == NULL)
					error("Runtime error: Write NULL error.");
				else
					for(int i = 0; data[i] != NULL; i++) 
						print_object(data[i]);

				for(int i = 0; data[i] != NULL; i++)
					object_dereference(data[i]);
				_free(data);
			} break;
			case OPERATION_TYPE_ADD: 
				for(int i = 0; op->data.operations[i] != NULL; i++)
					operation_exec(op->data.operations[i], env);
				break;
			case OPERATION_TYPE_SUB: 
				for(int i = 0; op->data.operations[i] != NULL; i++)
					operation_exec(op->data.operations[i], env);
				break;
			case OPERATION_TYPE_MUL:
				for(int i = 0; op->data.operations[i] != NULL; i++)
					operation_exec(op->data.operations[i], env);
				break;
			case OPERATION_TYPE_DIV: 
				for(int i = 0; op->data.operations[i] != NULL; i++)
					operation_exec(op->data.operations[i], env);
				break;
			case OPERATION_TYPE_MOD: 
				for(int i = 0; op->data.operations[i] != NULL; i++)
					operation_exec(op->data.operations[i], env);
				break;
			case OPERATION_TYPE_NEG: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_POW: 
				for(int i = 0; op->data.operations[i] != NULL; i++)
					operation_exec(op->data.operations[i], env);
				break;
			case OPERATION_TYPE_AND: 
				for(int i = 0; op->data.operations[i] != NULL; i++)
					operation_exec(op->data.operations[i], env);
				break;
			case OPERATION_TYPE_OR: 
				for(int i = 0; op->data.operations[i] != NULL; i++)
					operation_exec(op->data.operations[i], env);
				break;
			case OPERATION_TYPE_XOR: 
				for(int i = 0; op->data.operations[i] != NULL; i++)
					operation_exec(op->data.operations[i], env);
				break;
			case OPERATION_TYPE_NOT: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_SQRT: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_CBRT: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_SIN: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_COS: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_TAN: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_ASIN: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_ACOS: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_ATAN: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_SINH: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_COSH: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_TANH: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_ASINH: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_ACOSH: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_ATANH: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_TRUNC: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_FLOOR: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_CEIL: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_ROUND: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_RAND: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_LEN: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_EQU: 
				operation_exec(op->data.operations[0], env);
				operation_exec(op->data.operations[1], env);
				break;
			case OPERATION_TYPE_GEQ: 
				operation_exec(op->data.operations[0], env);
				operation_exec(op->data.operations[1], env);
				break;
			case OPERATION_TYPE_LEQ: 
				operation_exec(op->data.operations[0], env);
				operation_exec(op->data.operations[1], env);
				break;
			case OPERATION_TYPE_GTR: 
				operation_exec(op->data.operations[0], env);
				operation_exec(op->data.operations[1], env);
				break;
			case OPERATION_TYPE_LES: 
				operation_exec(op->data.operations[0], env);
				operation_exec(op->data.operations[1], env);
				break;
			case OPERATION_TYPE_DIC: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_FIND: 
				operation_exec(op->data.operations[0], env);
				operation_exec(op->data.operations[1], env);
				break;
			case OPERATION_TYPE_SPLIT: 
				operation_exec(op->data.operations[0], env);
				operation_exec(op->data.operations[1], env);
				break;
			case OPERATION_TYPE_ABS: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_SCOPE: 
				environment_add_scope(env);
				operation_exec(op->data.operations[0], env); 
				environment_remove_scope(env);
				break;
			case OPERATION_TYPE_IF: {
				object_t** cond = operation_result(op->data.operations[0], env);
				
				if(cond == NULL)
					error("Runtime error: If NULL error.");
				if(cond[1] != NULL)
					error("Runtime error: If non-scalar condition error.");

				if(is_true(cond[0]))
					operation_exec(op->data.operations[1], env);

				object_dereference(cond[0]);
				_free(cond);
			} break;
			case OPERATION_TYPE_IFELSE: {
				object_t** cond = operation_result(op->data.operations[0], env);
				
				if(cond == NULL)
					error("Runtime error: If-else NULL error.");
				if(cond[1] != NULL)
					error("Runtime error: If-else non-scalar condition error.");

				if(is_true(cond[0]))
					operation_exec(op->data.operations[1], env);
				else
					operation_exec(op->data.operations[2], env);

				object_dereference(cond[0]);
				_free(cond);
			} break;
			case OPERATION_TYPE_WHILE: {
				object_t** cond = operation_result(op->data.operations[0], env);
				
				if(cond == NULL)
					error("Runtime error: while NULL error.");
				if(cond[1] != NULL)
					error("Runtime error: while non-scalar condition error.");

				while(is_true(cond[0]))
				{
					operation_exec(op->data.operations[1], env);

					object_dereference(cond[0]);
					_free(cond);
					cond = operation_result(op->data.operations[0], env);
					if(cond == NULL)
						error("Runtime error: while NULL error.");
					if(cond[1] != NULL)
						error("Runtime error: while non-scalar condition error.");
				}
			} break;
			case OPERATION_TYPE_IN_STRUCT: {
				object_t** stc = operation_result(op->data.operations[0], env);

				if(stc == NULL)
					error("Runtime error: Struct NULL error.");
				
				for(int i = 0; stc[i] != NULL; i++)
					if(stc[i]->type != OBJECT_TYPE_STRUCT)
						error("Runtime error: Struct type error.");
					else
						struct_exec(stc[i]->data.stc, op->data.operations[1]);
					

				for(int i = 0; stc[i] != NULL; i++)
					object_dereference(stc[i]);
				_free(stc);
			} break;
			case OPERATION_TYPE_LOCAL: {
				size_t prev_limit = env->local_mode_limit;
				environment_set_local_mode(env, env->count-1);
				operation_exec(op->data.operations[0], env);
				environment_set_local_mode(env, prev_limit);
			} break;
			case OPERATION_TYPE_GLOBAL: {
				size_t prev_limit = env->local_mode_limit;
				environment_set_local_mode(env, 0);
				operation_exec(op->data.operations[0], env);
				environment_set_local_mode(env, prev_limit);
			} break;
			case OPERATION_TYPE_COPY: operation_exec(op->data.operations[0], env); break;
			case OPERATION_TYPE_FOR: {
				operation_exec(op->data.operations[0], env);
				object_t** cond = operation_result(op->data.operations[1], env);
				
				if(cond == NULL)
					error("Runtime error: while NULL error.");
				if(cond[1] != NULL)
					error("Runtime error: while non-scalar condition error.");

				while(is_true(cond[0]))
				{
					operation_exec(op->data.operations[3], env);
					operation_exec(op->data.operations[2], env);

					object_dereference(cond[1]);
					_free(cond);
					cond = operation_result(op->data.operations[1], env);
					if(cond == NULL)
						error("Runtime error: while NULL error.");
					if(cond[1] != NULL)
						error("Runtime error: while non-scalar condition error.");
				}
			} break;
		}
	}
}

object_t** operation_result(operation_t* op, environment_t* env) {
	if(check_for_stackoverflow())
		error("Runtime error: Stack overflow.");

	object_t** ret = NULL;
	
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
					_free(ret);
					ret = tmp;
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

				if(keys == NULL || vals == NULL)
					error("Runtime error: Pair NULL error.");

				size_t keys_len = 0;
				while(keys[keys_len] != NULL) keys_len++;
				size_t vals_len = 0;
				while(vals[vals_len] != NULL) vals_len++;

				if(keys_len > 1 && vals_len > 1)
					if(keys_len != vals_len)
						error("Runtime error: Pair symmetry error");
					else {
						ret = (object_t**)_alloc(sizeof(object_t*)*(vals_len + 1));

						for(int i = 0; i < vals_len; i++) {
							ret[i] = object_create_pair(pair_create(keys[i], vals[i]));
							object_reference(ret[i]);
						}
						ret[vals_len] = NULL;
					}
				else if(keys_len < vals_len) {
					ret = (object_t**)_alloc(sizeof(object_t*)*(vals_len + 1));

					for(int i = 0; i < vals_len; i++) {
						ret[i] = object_create_pair(pair_create(keys[0], vals[i]));
						object_reference(ret[i]);
					}
					ret[vals_len] = NULL;
				}
				else {
					ret = (object_t**)_alloc(sizeof(object_t*)*(keys_len + 1));

					for(int i = 0; i < keys_len; i++) {
						ret[i] = object_create_pair(pair_create(keys[i], vals[0]));
						object_reference(ret[i]);
					}
					ret[keys_len] = NULL;
				}

				for(int i = 0; i < keys_len; i++)
					object_dereference(keys[i]);
				_free(keys);
				for(int i = 0; i < vals_len; i++)
					object_dereference(vals[i]);
				_free(vals);
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
				object_t*** assign_loc = operation_var(op->data.operations[0], env);
				object_t**	assign_val = operation_result(op->data.operations[1], env);

				if(assign_loc == NULL || assign_val == NULL)
					error("Runtime error: Assignment NULL error.");
				else
					for(int i = 0; assign_loc[i] != NULL && assign_val[i] != NULL; i++) {
						object_dereference(*(assign_loc[i]));
						*(assign_loc[i]) = assign_val[i];
						object_reference(*(assign_loc[i]));
					}
					
				_free(assign_loc);
				ret = assign_val;
			} break;
			case OPERATION_TYPE_PROC: {
				int i;
				for(i = 0; op->data.operations[i+1] != NULL; i++)
					operation_exec(op->data.operations[i], env);
				ret = operation_result(op->data.operations[i], env);
			} break;
			case OPERATION_TYPE_STRUCT: {
				ret = (object_t**)_alloc(sizeof(object_t*)*2);
				ret[0] = object_create_struct(struct_create());
				object_reference(ret[0]);
				ret[1] = NULL;
				string_t* name_self = string_create("self");
				environment_write(ret[0]->data.stc, name_self, ret[0]);
				object_dereference(ret[0]);	// Since the object contains itfels derefrencing helps to prevent loops (Better garbage collector is required)
				string_free(name_self);
				struct_exec(ret[0]->data.stc, op->data.operations[0]);
			} break;
			case OPERATION_TYPE_O_LIST: {
				size_t num_op = 0;
				while(op->data.operations[num_op] != NULL) num_op++;

				object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

				size_t num_ret = 0;
				for(int i = 0; i < num_op; i++) {
					vals[i] = operation_result(op->data.operations[i], env);
					if(vals[i] == NULL)
						error("Runtime error: Open list NULL error.");
					for(int j = 0; vals[i][j] != NULL; j++)
						num_ret++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
				num_ret = 0;
				for(int i = 0; i < num_op; i++) {
					for(int j = 0; vals[i][j] != NULL; j++)
						ret[num_ret++] = vals[i][j];
				}
				ret[num_ret] = NULL;
				
				for(int i = 0; i < num_op; i++)
					_free(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_LIST: {
				object_t** vals = operation_result(op->data.operations[0], env);

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
			} break;
			case OPERATION_TYPE_INDEX: {
				object_t** data = operation_result(op->data.operations[0], env);
				object_t** index = operation_result(op->data.operations[1], env);

				if(data == NULL || index == NULL)
					error("Runtime error: Indexing NULL error.");

				size_t num_data = 0;
				while(data[num_data] != NULL) num_data++;
				size_t num_index = 0;
				while(index[num_index] != NULL) num_index++;
				ret = (object_t**)_alloc(sizeof(object_t*)*(num_data*num_index+1));

				for(int i = 0; i < num_data; i++) 
					for(int j = 0; j < num_index; j++) {
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
									ret[i*num_index+j] = object_create_none();
								else
									ret[i*num_index+j] = object_create_list(list);
								object_reference(ret[i*num_index+j]);

							} else
								error("Runtime error: Indexing type error."); 

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
									ret[i*num_index+j] = object_create_none();
								else
									ret[i*num_index+j] = object_create_string(str);
								object_reference(ret[i*num_index+j]);

							} else
								error("Runtime error: Indexing type error."); 

						} else
							error("Runtime error: Indexing type error."); 
					}
				
				ret[num_data*num_index] = NULL;

				for(int i = 0; i < num_data; i++)
					object_dereference(data[i]);
				_free(data);
				for(int i = 0; i < num_index; i++)
					object_dereference(index[i]);
				_free(index);
			} break;
			case OPERATION_TYPE_EXEC: {
				object_t** function = operation_result(op->data.operations[0], env);
				object_t** parameter = operation_result(op->data.operations[1], env);

				if(function == NULL)
					error("Runtime error: Function NULL error.");
				
				size_t num_func = 0;
				while(function[num_func] != NULL) num_func++;

				object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_func);

				size_t num_ret = 0;
				for(int i = 0; i < num_func; i++) {
					vals[i] = function_result(function[i]->data.func, parameter, env);
					if(vals[i] == NULL)
						error("Runtime error: Function NULL error.");
					for(int j = 0; vals[i][j] == NULL; j++)
						num_ret++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
				num_ret = 0;
				for(int i = 0; i < num_func; i++)
					for(int j = 0; vals[i][j] != NULL; j++) {
						ret[num_ret] = vals[i][j];
						num_ret++;
					}
				ret[num_ret] = NULL;

				for(int i = 0; i < num_func; i++)
					_free(vals[i]);
				_free(vals);
				for(int i = 0; i < num_func; i++)
					object_dereference(function[i]);
				_free(function);
				if(parameter != NULL) {
					for(int i = 0; parameter[i] != NULL; i++)
						object_dereference(parameter[i]);
					_free(parameter);
				}
			} break;
			case OPERATION_TYPE_TO_NUM: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: To_num NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) num_vals++;

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
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
						case OBJECT_TYPE_STRUCT: error("Runtime error: To_num type error."); break;
					}
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;

				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_TO_BOOL: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: To_bool NULL error.");
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
			} break;
			case OPERATION_TYPE_TO_STR: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: To_bool NULL error.");
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

				if(data == NULL)
					error("Runtime error: Write NULL error.");
				else
					for(int i = 0; data[i] != NULL; i++) 
						print_object(data[i]);

				for(int i = 0; data[i] != NULL; i++)
					object_dereference(data[i]);
				_free(data);
			} break;
			case OPERATION_TYPE_ADD: {
				size_t num_op = 0;
				while(op->data.operations[num_op] != NULL) num_op++;

				object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

				size_t num_ret = 1;
				for(int i = 0; i < num_op; i++) {
					vals[i] = operation_result(op->data.operations[i], env);
					if(vals[i] == NULL)
						error("Runtime error: Addition NULL error.");
					size_t tmp_size = 0;
					while(vals[i][tmp_size] != NULL) {
						tmp_size++;
					}
					if(num_ret == 1 && tmp_size != 1)
						num_ret = tmp_size;
					else if(num_ret != tmp_size && tmp_size != 1)
						error("Runtime error: Addition symmetry error.");
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
				for(int i = 0; i < num_ret; i++) {	
					if(vals[0][1] == NULL)
						ret[i] = vals[0][0];
					else
						ret[i] = vals[0][i];
					object_reference(ret[i]);
					object_t* tmp;

					for(int j = 1; j < num_op; j++) {
						if(vals[j][1] == NULL)
							tmp = object_add(ret[i], vals[j][0]);
						else
							tmp = object_add(ret[i], vals[j][i]);
						object_dereference(ret[i]);
						ret[i] = tmp;
						object_reference(tmp);
					}
				}
				ret[num_ret] = NULL;

				for(int i = 0; i < num_op; i++) {
					for(int j = 0; vals[i][j] != NULL; j++)
						object_dereference(vals[i][j]);
					_free(vals[i]);
				}
				_free(vals);
			} break;
			case OPERATION_TYPE_SUB: {
				size_t num_op = 0;
				while(op->data.operations[num_op] != NULL) num_op++;

				object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

				size_t num_ret = 1;
				for(int i = 0; i < num_op; i++) {
					vals[i] = operation_result(op->data.operations[i], env);
					if(vals[i] == NULL)
						error("Runtime error: Subtraction NULL error.");
					size_t tmp_size = 0;
					while(vals[i][tmp_size] != NULL) {
						if(vals[i][tmp_size]->type != OBJECT_TYPE_NUMBER)
							error("Runtime error: Subtraction type error.");
						tmp_size++;
					}
					if(num_ret == 1 && tmp_size != 1)
						num_ret = tmp_size;
					else if(num_ret != tmp_size && tmp_size != 1)
						error("Runtime error: Subtraction symmetry error.");
				}

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

				for(int i = 0; i < num_op; i++) {
					for(int j = 0; vals[i][j] != NULL; j++)
						object_dereference(vals[i][j]);
					_free(vals[i]);
				}
				_free(vals);
			} break;
			case OPERATION_TYPE_MUL: {
				size_t num_op = 0;
				while(op->data.operations[num_op] != NULL) num_op++;

				object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

				size_t num_ret = 1;
				for(int i = 0; i < num_op; i++) {
					vals[i] = operation_result(op->data.operations[i], env);
					if(vals[i] == NULL)
						error("Runtime error: Multiplication NULL error.");
					size_t tmp_size = 0;
					while(vals[i][tmp_size] != NULL) {
						tmp_size++;
					}
					if(num_ret == 1 && tmp_size != 1)
						num_ret = tmp_size;
					else if(num_ret != tmp_size && tmp_size != 1)
						error("Runtime error: Multiplication symmetry error.");
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
				for(int i = 0; i < num_ret; i++) {	
					if(vals[0][1] == NULL)
						ret[i] = vals[0][0];
					else
						ret[i] = vals[0][i];
					object_reference(ret[i]);
					object_t* tmp;

					for(int j = 1; j < num_op; j++) {
						if(vals[j][1] == NULL)
							tmp = object_mul(ret[i], vals[j][0]);
						else
							tmp = object_mul(ret[i], vals[j][i]);
						object_dereference(ret[i]);
						ret[i] = tmp;
						object_reference(tmp);
					}
				}
				ret[num_ret] = NULL;

				for(int i = 0; i < num_op; i++) {
					for(int j = 0; vals[i][j] != NULL; j++)
						object_dereference(vals[i][j]);
					_free(vals[i]);
				}
				_free(vals);
			} break;
			case OPERATION_TYPE_DIV: {
				size_t num_op = 0;
				while(op->data.operations[num_op] != NULL) num_op++;

				object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

				size_t num_ret = 1;
				for(int i = 0; i < num_op; i++) {
					vals[i] = operation_result(op->data.operations[i], env);
					if(vals[i] == NULL)
						error("Runtime error: Division NULL error.");
					size_t tmp_size = 0;
					while(vals[i][tmp_size] != NULL) {
						if(vals[i][tmp_size]->type != OBJECT_TYPE_NUMBER)
							error("Runtime error: Division type error.");
						tmp_size++;
					}
					if(num_ret == 1 && tmp_size != 1)
						num_ret = tmp_size;
					else if(num_ret != tmp_size && tmp_size != 1)
						error("Runtime error: Division symmetry error.");
				}

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

				for(int i = 0; i < num_op; i++) {
					for(int j = 0; vals[i][j] != NULL; j++)
						object_dereference(vals[i][j]);
					_free(vals[i]);
				}
				_free(vals);
			} break;
			case OPERATION_TYPE_MOD: {
				size_t num_op = 0;
				while(op->data.operations[num_op] != NULL) num_op++;

				object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

				size_t num_ret = 1;
				for(int i = 0; i < num_op; i++) {
					vals[i] = operation_result(op->data.operations[i], env);
					if(vals[i] == NULL)
						error("Runtime error: Modulo NULL error.");
					size_t tmp_size = 0;
					while(vals[i][tmp_size] != NULL) {
						if(vals[i][tmp_size]->type != OBJECT_TYPE_NUMBER)
							error("Runtime error: Modulo type error.");
						tmp_size++;
					}
					if(num_ret == 1 && tmp_size != 1)
						num_ret = tmp_size;
					else if(num_ret != tmp_size && tmp_size != 1)
						error("Runtime error: Modulo symmetry error.");
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
				for(int i = 0; i < num_ret; i++) {
					long ret_part;
					if(vals[0][1] == NULL) {
						if(vals[0][0]->data.number != round(vals[0][0]->data.number))
							error("Runtime error: Modulo non-integer Error");
						ret_part = (long)(vals[0][0]->data.number);
					} else {
						if(vals[0][i]->data.number != round(vals[0][i]->data.number))
							error("Runtime error: Modulo non-integer Error");
						ret_part = (long)(vals[0][i]->data.number);
					}

					for(int j = 1; j < num_op; j++) {
						if(vals[j][1] == NULL) {
							if(vals[j][0]->data.number != round(vals[j][0]->data.number))
								error("Runtime error: Modulo non-integer Error");
							ret_part %= (long)(vals[j][0]->data.number);
						} else {
							if(vals[j][i]->data.number != round(vals[j][i]->data.number))
								error("Runtime error: Modulo non-integer Error");
							ret_part %= (long)(vals[j][i]->data.number);
						}
					}

					ret[i] = object_create_number(ret_part);
					object_reference(ret[i]);
				}
				ret[num_ret] = NULL;

				for(int i = 0; i < num_op; i++) {
					for(int j = 0; vals[i][j] != NULL; j++)
						object_dereference(vals[i][j]);
					_free(vals[i]);
				}
				_free(vals);
			} break;
			case OPERATION_TYPE_NEG: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Negate NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Negate type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(-vals[i]->data.number);
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_POW: {
				size_t num_op = 0;
				while(op->data.operations[num_op] != NULL) num_op++;

				object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

				size_t num_ret = 1;
				for(int i = 0; i < num_op; i++) {
					vals[i] = operation_result(op->data.operations[i], env);
					if(vals[i] == NULL)
						error("Runtime error: Power NULL error.");
					size_t tmp_size = 0;
					while(vals[i][tmp_size] != NULL) {
						if(vals[i][tmp_size]->type != OBJECT_TYPE_NUMBER)
							error("Runtime error: Power type error.");
						tmp_size++;
					}
					if(num_ret == 1 && tmp_size != 1)
						num_ret = tmp_size;
					else if(num_ret != tmp_size && tmp_size != 1)
						error("Runtime error: Power symmetry error.");
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
				for(int i = 0; i < num_ret; i++) {
					number_t ret_part;
					if(vals[0][1] == NULL)
						ret_part = (int)(vals[0][0]->data.number);
					else 
						ret_part = (int)(vals[0][i]->data.number);
					

					for(int j = num_op-1; j >= 0; j--) {
						if(vals[j][1] == NULL)
							ret_part = pow(vals[j][0]->data.number, ret_part);
						else
							ret_part = pow(vals[j][i]->data.number, ret_part);
					}

					ret[i] = object_create_number(ret_part);
					object_reference(ret[i]);
				}
				ret[num_ret] = NULL;

				for(int i = 0; i < num_op; i++) {
					for(int j = 0; vals[i][j] != NULL; j++)
						object_dereference(vals[i][j]);
					_free(vals[i]);
				}
				_free(vals);
			} break;
			case OPERATION_TYPE_AND: {
				size_t num_op = 0;
				while(op->data.operations[num_op] != NULL) num_op++;

				object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

				size_t num_ret = 1;
				for(int i = 0; i < num_op; i++) {
					vals[i] = operation_result(op->data.operations[i], env);
					if(vals[i] == NULL)
						error("Runtime error: And NULL error.");
					size_t tmp_size = 0;
					while(vals[i][tmp_size] != NULL) {
						if(vals[i][tmp_size]->type != OBJECT_TYPE_BOOL)
							error("Runtime error: And type error.");
						tmp_size++;
					}
					if(num_ret == 1 && tmp_size != 1)
						num_ret = tmp_size;
					else if(num_ret != tmp_size && tmp_size != 1)
						error("Runtime error: And symmetry error.");
				}

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

				for(int i = 0; i < num_op; i++) {
					for(int j = 0; vals[i][j] != NULL; j++)
						object_dereference(vals[i][j]);
					_free(vals[i]);
				}
				_free(vals);
			} break;
			case OPERATION_TYPE_OR: {
				size_t num_op = 0;
				while(op->data.operations[num_op] != NULL) num_op++;

				object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

				size_t num_ret = 1;
				for(int i = 0; i < num_op; i++) {
					vals[i] = operation_result(op->data.operations[i], env);
					if(vals[i] == NULL)
						error("Runtime error: Or NULL error.");
					size_t tmp_size = 0;
					while(vals[i][tmp_size] != NULL) {
						if(vals[i][tmp_size]->type != OBJECT_TYPE_BOOL)
							error("Runtime error: Or type error.");
						tmp_size++;
					}
					if(num_ret == 1 && tmp_size != 1)
						num_ret = tmp_size;
					else if(num_ret != tmp_size && tmp_size != 1)
						error("Runtime error: Or symmetry error.");
				}

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

				for(int i = 0; i < num_op; i++) {
					for(int j = 0; vals[i][j] != NULL; j++)
						object_dereference(vals[i][j]);
					_free(vals[i]);
				}
				_free(vals);
			} break;
			case OPERATION_TYPE_XOR: {
				size_t num_op = 0;
				while(op->data.operations[num_op] != NULL) num_op++;

				object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_op);

				size_t num_ret = 1;
				for(int i = 0; i < num_op; i++) {
					vals[i] = operation_result(op->data.operations[i], env);
					if(vals[i] == NULL)
						error("Runtime error: Xor NULL error.");
					size_t tmp_size = 0;
					while(vals[i][tmp_size] != NULL) {
						if(vals[i][tmp_size]->type != OBJECT_TYPE_BOOL)
							error("Runtime error: Xor type error.");
						tmp_size++;
					}
					if(num_ret == 1 && tmp_size != 1)
						num_ret = tmp_size;
					else if(num_ret != tmp_size && tmp_size != 1)
						error("Runtime error: Xor symmetry error.");
				}

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

				for(int i = 0; i < num_op; i++) {
					for(int j = 0; vals[i][j] != NULL; j++)
						object_dereference(vals[i][j]);
					_free(vals[i]);
				}
				_free(vals);
			} break;
			case OPERATION_TYPE_NOT: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Not NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_BOOL)
						error("Runtime error: Not type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_boolean(!vals[i]->data.boolean);
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_SQRT: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Sqrt NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Sqrt type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(sqrt(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_CBRT: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Cbrt NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Cbrt type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(cbrt(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_SIN: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Sin NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Sin type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(sin(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_COS: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Cos NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Cos type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(cos(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_TAN: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Tan NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Tan type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(tan(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_ASIN: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Asin NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Asin type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(asin(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_ACOS: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Acos NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Acos type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(acos(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_ATAN: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Atan NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Atan type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(atan(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_SINH: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Sinh NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Sinh type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(sinh(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_COSH: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Cosh NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Cosh type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(cosh(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_TANH: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Tanh NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Tanh type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(tanh(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_ASINH: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Asinh NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Asinh type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(asinh(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_ACOSH: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Acosh NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Acosh type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(acosh(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_ATANH: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Atanh NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Atanh type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(atanh(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_TRUNC: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Trunc NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Trunc type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(trunc(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_FLOOR: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Floor NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Floor type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(floor(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_CEIL: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Ceil NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Ceil type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(ceil(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_ROUND: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Round NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Round type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(round(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_RAND:
				ret = (object_t**)_alloc(sizeof(object_t*)*2);
				ret[0] = object_create_number((double)rand()/RAND_MAX);
				object_reference(ret[0]);
				ret[1] = NULL;
				break;
			case OPERATION_TYPE_LEN: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Len NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					if(vals[i]->type == OBJECT_TYPE_LIST) {
						ret[i] = object_create_number(list_size(vals[i]->data.list));
					} else if (vals[i]->type == OBJECT_TYPE_STRING) {
						ret[i] = object_create_number(string_length(vals[i]->data.string));
					} else
						error("Runtime error: Len type error.");
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_EQU: {
				object_t** left = operation_result(op->data.operations[0], env);
				object_t** right = operation_result(op->data.operations[1], env);

				if(left == NULL || right == NULL)
					error("Runtime error: Equ NULL error.");

				size_t left_len = 0;
				while(left[left_len] != NULL) left_len++;
				size_t right_len = 0;
				while(right[right_len] != NULL) right_len++;

				if(left_len > 1 && right_len > 1)
					if(left_len != right_len)
						error("Runtime error: Equ symmetry error");
					else {
						ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

						for(int i = 0; i < right_len; i++) {
							ret[i] = object_create_boolean(object_equ(left[i], right[i]));
							object_reference(ret[i]);
						}
						ret[right_len] = NULL;
					}
				else if(left_len < right_len) {
					ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

					for(int i = 0; i < right_len; i++) {
						ret[i] = object_create_boolean(object_equ(left[0], right[i]));
						object_reference(ret[i]);
					}
					ret[right_len] = NULL;
				}
				else {
					ret = (object_t**)_alloc(sizeof(object_t*)*(left_len + 1));

					for(int i = 0; i < left_len; i++) {
						ret[i] = object_create_boolean(object_equ(left[i], right[0]));
						object_reference(ret[i]);
					}
					ret[left_len] = NULL;
				}

				for(int i = 0; i < left_len; i++)
					object_dereference(left[i]);
				_free(left);
				for(int i = 0; i < right_len; i++)
					object_dereference(right[i]);
				_free(right);
			} break;
			case OPERATION_TYPE_GEQ: {
				object_t** left = operation_result(op->data.operations[0], env);
				object_t** right = operation_result(op->data.operations[1], env);

				if(left == NULL || right == NULL)
					error("Runtime error: Compare NULL error.");

				size_t left_len = 0;
				while(left[left_len] != NULL) left_len++;
				size_t right_len = 0;
				while(right[right_len] != NULL) right_len++;

				if(left_len > 1 && right_len > 1)
					if(left_len != right_len)
						error("Runtime error: Compare symmetry error");
					else {
						ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

						for(int i = 0; i < right_len; i++) {
							if(left[i]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER)
								error("Runtime error: Compare type error");
							ret[i] = object_create_boolean(left[i]->data.number >= right[i]->data.number);
							object_reference(ret[i]);
						}
						ret[right_len] = NULL;
					}
				else if(left_len < right_len) {
					ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

					for(int i = 0; i < right_len; i++) {
						if(left[0]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER)
							error("Runtime error: Compare type error");
						ret[i] = object_create_boolean(left[0]->data.number >= right[i]->data.number);
						object_reference(ret[i]);
					}
					ret[right_len] = NULL;
				}
				else {
					ret = (object_t**)_alloc(sizeof(object_t*)*(left_len + 1));

					for(int i = 0; i < left_len; i++) {
						if(left[i]->type != OBJECT_TYPE_NUMBER || right[0]->type != OBJECT_TYPE_NUMBER)
							error("Runtime error: Compare type error");
						ret[i] = object_create_boolean(left[i]->data.number >= right[0]->data.number);
						object_reference(ret[i]);
					}
					ret[left_len] = NULL;
				}

				for(int i = 0; i < left_len; i++)
					object_dereference(left[i]);
				_free(left);
				for(int i = 0; i < right_len; i++)
					object_dereference(right[i]);
				_free(right);
			} break;
			case OPERATION_TYPE_LEQ: {
				object_t** left = operation_result(op->data.operations[0], env);
				object_t** right = operation_result(op->data.operations[1], env);

				if(left == NULL || right == NULL)
					error("Runtime error: Compare NULL error.");

				size_t left_len = 0;
				while(left[left_len] != NULL) left_len++;
				size_t right_len = 0;
				while(right[right_len] != NULL) right_len++;

				if(left_len > 1 && right_len > 1)
					if(left_len != right_len)
						error("Runtime error: Compare symmetry error");
					else {
						ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

						for(int i = 0; i < right_len; i++) {
							if(left[i]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER)
								error("Runtime error: Compare type error");
							ret[i] = object_create_boolean(left[i]->data.number <= right[i]->data.number);
							object_reference(ret[i]);
						}
						ret[right_len] = NULL;
					}
				else if(left_len < right_len) {
					ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

					for(int i = 0; i < right_len; i++) {
						if(left[0]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER)
							error("Runtime error: Compare type error");
						ret[i] = object_create_boolean(left[0]->data.number <= right[i]->data.number);
						object_reference(ret[i]);
					}
					ret[right_len] = NULL;
				}
				else {
					ret = (object_t**)_alloc(sizeof(object_t*)*(left_len + 1));

					for(int i = 0; i < left_len; i++) {
						if(left[i]->type != OBJECT_TYPE_NUMBER || right[0]->type != OBJECT_TYPE_NUMBER)
							error("Runtime error: Compare type error");
						ret[i] = object_create_boolean(left[i]->data.number <= right[0]->data.number);
						object_reference(ret[i]);
					}
					ret[left_len] = NULL;
				}

				for(int i = 0; i < left_len; i++)
					object_dereference(left[i]);
				_free(left);
				for(int i = 0; i < right_len; i++)
					object_dereference(right[i]);
				_free(right);
			} break;
			case OPERATION_TYPE_GTR: {
				object_t** left = operation_result(op->data.operations[0], env);
				object_t** right = operation_result(op->data.operations[1], env);

				if(left == NULL || right == NULL)
					error("Runtime error: Compare NULL error.");

				size_t left_len = 0;
				while(left[left_len] != NULL) left_len++;
				size_t right_len = 0;
				while(right[right_len] != NULL) right_len++;

				if(left_len > 1 && right_len > 1)
					if(left_len != right_len)
						error("Runtime error: Compare symmetry error");
					else {
						ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

						for(int i = 0; i < right_len; i++) {
							if(left[i]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER)
								error("Runtime error: Compare type error");
							ret[i] = object_create_boolean(left[i]->data.number > right[i]->data.number);
							object_reference(ret[i]);
						}
						ret[right_len] = NULL;
					}
				else if(left_len < right_len) {
					ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

					for(int i = 0; i < right_len; i++) {
						if(left[0]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER)
							error("Runtime error: Compare type error");
						ret[i] = object_create_boolean(left[0]->data.number > right[i]->data.number);
						object_reference(ret[i]);
					}
					ret[right_len] = NULL;
				}
				else {
					ret = (object_t**)_alloc(sizeof(object_t*)*(left_len + 1));

					for(int i = 0; i < left_len; i++) {
						if(left[i]->type != OBJECT_TYPE_NUMBER || right[0]->type != OBJECT_TYPE_NUMBER)
							error("Runtime error: Compare type error");
						ret[i] = object_create_boolean(left[i]->data.number > right[0]->data.number);
						object_reference(ret[i]);
					}
					ret[left_len] = NULL;
				}

				for(int i = 0; i < left_len; i++)
					object_dereference(left[i]);
				_free(left);
				for(int i = 0; i < right_len; i++)
					object_dereference(right[i]);
				_free(right);
			} break;
			case OPERATION_TYPE_LES: {
				object_t** left = operation_result(op->data.operations[0], env);
				object_t** right = operation_result(op->data.operations[1], env);

				if(left == NULL || right == NULL)
					error("Runtime error: Compare NULL error.");

				size_t left_len = 0;
				while(left[left_len] != NULL) left_len++;
				size_t right_len = 0;
				while(right[right_len] != NULL) right_len++;

				if(left_len > 1 && right_len > 1)
					if(left_len != right_len)
						error("Runtime error: Compare symmetry error");
					else {
						ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

						for(int i = 0; i < right_len; i++) {
							if(left[i]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER)
								error("Runtime error: Compare type error");
							ret[i] = object_create_boolean(left[i]->data.number < right[i]->data.number);
							object_reference(ret[i]);
						}
						ret[right_len] = NULL;
					}
				else if(left_len < right_len) {
					ret = (object_t**)_alloc(sizeof(object_t*)*(right_len + 1));

					for(int i = 0; i < right_len; i++) {
						if(left[0]->type != OBJECT_TYPE_NUMBER || right[i]->type != OBJECT_TYPE_NUMBER)
							error("Runtime error: Compare type error");
						ret[i] = object_create_boolean(left[0]->data.number < right[i]->data.number);
						object_reference(ret[i]);
					}
					ret[right_len] = NULL;
				}
				else {
					ret = (object_t**)_alloc(sizeof(object_t*)*(left_len + 1));

					for(int i = 0; i < left_len; i++) {
						if(left[i]->type != OBJECT_TYPE_NUMBER || right[0]->type != OBJECT_TYPE_NUMBER)
							error("Runtime error: Compare type error");
						ret[i] = object_create_boolean(left[i]->data.number < right[0]->data.number);
						object_reference(ret[i]);
					}
					ret[left_len] = NULL;
				}

				for(int i = 0; i < left_len; i++)
					object_dereference(left[i]);
				_free(left);
				for(int i = 0; i < right_len; i++)
					object_dereference(right[i]);
				_free(right);
			} break;
			case OPERATION_TYPE_DIC: {
				object_t** vals = operation_result(op->data.operations[0], env);
	
				dictionary_t* dic;

				if(vals != NULL) {
					size_t num_vals = 0;
					while(vals[num_vals] != NULL) num_vals++;

					dic = dictionary_create_sized(num_vals*2);

					for(int i = 0; i < num_vals; i++) {
						if(vals[i]->type != OBJECT_TYPE_PAIR)
							error("Runtime error: Dictionary type error.");
						dictionary_put(dic, vals[i]->data.pair->key, vals[i]->data.pair->value);
					}
					for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
					_free(vals);
				} else {
					dic = dictionary_create();
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*2);
				ret[0] = object_create_dictionary(dic);
				object_reference(ret[0]);
				ret[1] = NULL;
			} break;
			case OPERATION_TYPE_FIND: {
				object_t** left = operation_result(op->data.operations[0], env);
				object_t** right = operation_result(op->data.operations[1], env);

				if(left == NULL || right == NULL)
					error("Runtime error: Find NULL error.");

				size_t left_len = 0;
				while(left[left_len] != NULL) left_len++;
				size_t right_len = 0;
				while(right[right_len] != NULL) right_len++;

				ret = (object_t**)_alloc(sizeof(object_t*)*(left_len*right_len+1));
				for(int i = 0; i < left_len; i++)
					for(int j = 0; j < right_len; j++) {
						switch(left[i]->type) {
							case OBJECT_TYPE_LIST: {
								pos_t pos = list_find(left[i]->data.list, right[j]);
								if(pos != -1)
									ret[i*right_len+j] = object_create_number((number_t)pos);
								else
									ret[i*right_len+j] = object_create_none();
							} break;
							case OBJECT_TYPE_STRING: 
								if(right[j]->type != OBJECT_TYPE_STRING)
									error("Runtime error: Find type error.");
								pos_t pos = string_find(left[i]->data.string, right[j]->data.string);
								if(pos != -1)
									ret[i*right_len+j] = object_create_number((number_t)pos);
								else
									ret[i*right_len+j] = object_create_none();
								break;
							case OBJECT_TYPE_DICTIONARY: ret[i*right_len+j] = object_create_boolean(dictionary_get(left[i]->data.dic, right[j]) != NULL); break;
							default: error("Runtime error: Find type error."); break;
						}
						object_reference(ret[i*right_len+j]);
					}
				ret[left_len*right_len] = NULL;

				for(int i = 0; i < left_len; i++)
					object_dereference(left[i]);
				_free(left);
				for(int i = 0; i < right_len; i++)
					object_dereference(right[i]);
				_free(right);
			} break;
			case OPERATION_TYPE_SPLIT: {
				object_t** src = operation_result(op->data.operations[0], env);
				object_t** splt = operation_result(op->data.operations[1], env);

				if(src == NULL || splt == NULL)
					error("Runtime error: Split NULL error.");

				size_t src_len = 0;
				while(src[src_len] != NULL) {
					if(src[src_len]->type != OBJECT_TYPE_STRING)
						error("Runtime error: Split type error.");
					src_len++;
				}
				size_t splt_len = 0;
				while(splt[splt_len] != NULL) {
					if(splt[splt_len]->type != OBJECT_TYPE_STRING)
						error("Runtime error: Split type error.");
					splt_len++;
				}

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


				for(int i = 0; i < src_len; i++)
					object_dereference(src[i]);
				_free(src);
				for(int i = 0; i < splt_len; i++)
					object_dereference(splt[i]);
				_free(splt);
			} break;
			case OPERATION_TYPE_ABS: {
				object_t** vals = operation_result(op->data.operations[0], env);
				if(vals == NULL)
					error("Runtime error: Abs NULL error.");
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) {
					if(vals[num_vals]->type != OBJECT_TYPE_NUMBER)
						error("Runtime error: Abs type error.");
					num_vals++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
					ret[i] = object_create_number(abs(vals[i]->data.number));
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;
				
				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_SCOPE:
				environment_add_scope(env);
				ret = operation_result(op->data.operations[0], env);
				environment_remove_scope(env);
			break;
			case OPERATION_TYPE_IF: {
				object_t** cond = operation_result(op->data.operations[0], env);
				
				if(cond == NULL)
					error("Runtime error: If NULL error.");
				if(cond[1] != NULL)
					error("Runtime error: If non-scalar condition error.");

				if(is_true(cond[0])) 
					ret = operation_result(op->data.operations[1], env);
				
				object_dereference(cond[0]);
				_free(cond);
			} break;
			case OPERATION_TYPE_IFELSE: {
				object_t** cond = operation_result(op->data.operations[0], env);
				
				if(cond == NULL)
					error("Runtime error: If-else NULL error.");
				if(cond[1] != NULL)
					error("Runtime error: If-else non-scalar condition error.");

				if(is_true(cond[0]))
					ret = operation_result(op->data.operations[1], env);
				else
					ret = operation_result(op->data.operations[2], env);

				object_dereference(cond[0]);
				_free(cond);
			} break;
			case OPERATION_TYPE_WHILE: {
				list_t list;
				list.data = NULL;
				list.size = 0;

				object_t** cond = operation_result(op->data.operations[0], env);
				
				if(cond == NULL)
					error("Runtime error: while NULL error.");
				if(cond[1] != NULL)
					error("Runtime error: while non-scalar condition error.");

				while(is_true(cond[0]))
				{
					object_t** tmp = operation_result(op->data.operations[1], env);

					for(int i = 0; tmp[i] != NULL; i++) {
						list_append(&list, tmp[i]);
						object_dereference(tmp[i]);
					}
					_free(tmp);

					object_dereference(cond[0]);
					_free(cond);
					cond = operation_result(op->data.operations[0], env);
					if(cond == NULL)
						error("Runtime error: while NULL error.");
					if(cond[1] != NULL)
						error("Runtime error: while non-scalar condition error.");
				}

				list_append(&list, NULL);
				ret = list.data;
			} break;
			case OPERATION_TYPE_IN_STRUCT: {
				object_t** stc = operation_result(op->data.operations[0], env);

				if(stc == NULL)
					error("Runtime error: Struct NULL error.");
				
				size_t num_stc = 0;
				while(stc[num_stc] != NULL) num_stc++;

				object_t*** vals = (object_t***)_alloc(sizeof(object_t**)*num_stc);

				size_t num_ret = 0;
				for(int i = 0; i < num_stc; i++) {
					if(stc[i]->type != OBJECT_TYPE_STRUCT)
						error("Runtime error: Struct type error.");
					vals[i] = struct_result(stc[i]->data.stc, op->data.operations[1]);
					if(vals[i] == NULL)
						error("Runtime error: Struct NULL error");
					for(int j = 0; vals[i][j] != NULL; j++)
						num_ret++;
				}

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_ret+1));
				num_ret = 0; 
				for(int i = 0; i < num_stc; i++) {
					for(int j = 0; vals[i][j] != NULL; j++) {
						ret[num_ret] = vals[j][i];
						num_ret++;
					}
				}
				ret[num_ret] = NULL;

				for(int i = 0; i < num_stc; i++)
					_free(vals[i]);
				_free(vals);
				for(int i = 0; i < num_stc; i++)
					object_dereference(stc[i]);
				_free(stc);
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
				
				size_t num_vals = 0;
				while(vals[num_vals] != NULL) num_vals++;

				ret = (object_t**)_alloc(sizeof(object_t*)*(num_vals+1));
				for(int i = 0; i < num_vals; i++) {
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
						case OBJECT_TYPE_STRUCT: error("Runtime error: Copy type error."); break;
					}
					object_reference(ret[i]);
				}
				ret[num_vals] = NULL;

				for(int i = 0; i < num_vals; i++)
					object_dereference(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_FOR: {
				list_t list;
				list.data = NULL;
				list.size = 0;

				operation_exec(op->data.operations[0], env);
				object_t** cond = operation_result(op->data.operations[1], env);
				
				if(cond == NULL)
					error("Runtime error: while NULL error.");
				if(cond[1] != NULL)
					error("Runtime error: while non-scalar condition error.");

				while(is_true(cond[0]))
				{
					object_t** tmp = operation_result(op->data.operations[3], env);
					operation_exec(op->data.operations[2], env);

					for(int i = 0; tmp[i] != NULL; i++) {
						list_append(&list, tmp[i]);
						object_dereference(tmp[i]);
					}
					_free(tmp);

					object_dereference(cond[1]);
					_free(cond);
					cond = operation_result(op->data.operations[1], env);
					if(cond == NULL)
						error("Runtime error: while NULL error.");
					if(cond[1] != NULL)
						error("Runtime error: while non-scalar condition error.");
				}

				list_append(&list, NULL);
				ret = list.data;
			} break;
		}
	}
	
	return ret;
}

object_t*** operation_var(operation_t* op, environment_t* env) {
	if(check_for_stackoverflow())
		error("Runtime error: Stack overflow.");

	object_t*** ret = NULL;

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
					_free(ret);
					ret = tmp;
				}
				break;
			case OPERATION_TYPE_BOOL: break;
			case OPERATION_TYPE_PAIR: break;
			case OPERATION_TYPE_FUNCTION: break;
			case OPERATION_TYPE_MACRO: break;
			case OPERATION_TYPE_ASSIGN: {
				object_t*** assign_loc = operation_var(op->data.operations[0], env);
				object_t**	assign_val = operation_result(op->data.operations[1], env);

				if(assign_loc == NULL || assign_val == NULL)
					error("Runtime error: Assignment NULL error.");
				else
					for(int i = 0; assign_loc[i] != NULL && assign_val[i] != NULL; i++) {
						object_dereference(*(assign_loc[i]));
						*(assign_loc[i]) = assign_val[i];
						object_reference(*(assign_loc[i]));
					}
					
				ret = assign_loc;
				for(int i = 0; assign_val[i] != NULL; i++)
					object_dereference(assign_val[i]);
				_free(assign_val);
			} break;
			case OPERATION_TYPE_PROC: {
				int i;
				for(i = 0; op->data.operations[i+1] != NULL; i++)
					operation_exec(op->data.operations[i], env);
				ret = operation_var(op->data.operations[i], env);
			} break;
			case OPERATION_TYPE_STRUCT: break;
			case OPERATION_TYPE_O_LIST: {
				size_t num_op = 0;
				while(op->data.operations[num_op] != NULL) num_op++;

				object_t**** vals = (object_t****)_alloc(sizeof(object_t***)*num_op);

				size_t num_ret = 0;
				for(int i = 0; i < num_op; i++) {
					vals[i] = operation_var(op->data.operations[i], env);
					if(vals[i] == NULL)
						error("Runtime error: Open list NULL error.");
					for(int j = 0; vals[i][j] != NULL; j++)
						num_ret++;
				}

				ret = (object_t***)_alloc(sizeof(object_t**)*(num_ret+1));

				num_ret = 0;
				for(int i = 0; i < num_op; i++) {
					for(int j = 0; vals[i][j] != NULL; j++)
						ret[num_ret++] = vals[i][j];
				}
				ret[num_ret] = NULL;
				
				for(int i = 0; i < num_op; i++)
					_free(vals[i]);
				_free(vals);
			} break;
			case OPERATION_TYPE_LIST: break;
			case OPERATION_TYPE_INDEX: {
				object_t** data = operation_result(op->data.operations[0], env);
				object_t** index = operation_result(op->data.operations[1], env);

				if(data == NULL || index == NULL)
					error("Runtime error: Indexing NULL error.");

				size_t num_data = 0;
				while(data[num_data] != NULL) num_data++;
				size_t num_index = 0;
				while(index[num_index] != NULL) num_index++;
				ret = (object_t***)_alloc(sizeof(object_t**)*(num_data*num_index+1));

				for(int i = 0; i < num_data; i++) 
					for(int j = 0; j < num_index; j++) {
						if(data[i]->type == OBJECT_TYPE_LIST) {

							if(index[j]->type == OBJECT_TYPE_NUMBER) {

								pos_t ind = (int)round(index[j]->data.number);
								if(ind < 0)
									ind += data[i]->data.list->size;
								object_t** obj = list_get_loc(data[i]->data.list, ind);
								if(obj == NULL)
									error("Runtime error: Indexing list out-of-range error.");
								else
									ret[i*num_index+j] = obj;

							} else
								error("Runtime error: Indexing type error."); 

						} else if (data[i]->type == OBJECT_TYPE_DICTIONARY) {

							ret[i*num_index+j] = dictionary_get_loc(data[i]->data.dic, index[j]);

						}  else
							error("Runtime error: Indexing type error."); 
					}
				
				ret[num_data*num_index] = NULL;

				for(int i = 0; i < num_data; i++)
					object_dereference(data[i]);
				_free(data);
				for(int i = 0; i < num_index; i++)
					object_dereference(index[i]);
				_free(index);
			} break;
			case OPERATION_TYPE_EXEC: break;
			case OPERATION_TYPE_TO_NUM: break;
			case OPERATION_TYPE_TO_BOOL: break;
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
				
				if(cond == NULL)
					error("Runtime error: If NULL error.");
				if(cond[1] != NULL)
					error("Runtime error: If non-scalar condition error.");

				if(is_true(cond[0])) 
					ret = operation_var(op->data.operations[1], env);
				
				object_dereference(cond[0]);
				_free(cond);
			} break;
			case OPERATION_TYPE_IFELSE: {
				object_t** cond = operation_result(op->data.operations[0], env);
				
				if(cond == NULL)
					error("Runtime error: If-else NULL error.");
				if(cond[1] != NULL)
					error("Runtime error: If-else non-scalar condition error.");

				if(is_true(cond[0]))
					ret = operation_var(op->data.operations[1], env);
				else
					ret = operation_var(op->data.operations[2], env);

				object_dereference(cond[0]);
				_free(cond);
			}break;
			case OPERATION_TYPE_WHILE: {
			} break;
			case OPERATION_TYPE_IN_STRUCT: {
				object_t** stc = operation_result(op->data.operations[0], env);

				if(stc == NULL)
					error("Runtime error: Struct NULL error.");
				
				size_t num_stc = 0;
				while(stc[num_stc] != NULL) num_stc++;

				object_t**** vals = (object_t****)_alloc(sizeof(object_t***)*num_stc);

				size_t num_ret = 0;
				for(int i = 0; i < num_stc; i++) {
					if(stc[i]->type != OBJECT_TYPE_STRUCT)
						error("Runtime error: Struct type error.");
					vals[i] = struct_var(stc[i]->data.stc, op->data.operations[1]);
					if(vals[i] == NULL)
						error("Runtime error: Struct NULL error");
					for(int j = 0; vals[i][j] != NULL; j++)
						num_ret++;
				}

				ret = (object_t***)_alloc(sizeof(object_t**)*(num_ret+1));
				num_ret = 0; 
				for(int i = 0; i < num_stc; i++) {
					for(int j = 0; vals[i][j] != NULL; j++) {
						ret[num_ret] = vals[j][i];
						num_ret++;
					}
				}
				ret[num_ret] = NULL;

				for(int i = 0; i < num_stc; i++)
					_free(vals[i]);
				_free(vals);
				for(int i = 0; i < num_stc; i++)
					object_dereference(stc[i]);
				_free(stc);
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
		}
		_free(op);
	}
}

// TODO:

id_t operation_id(operation_t* op) {
	id_t ret = 0;

	error("Runtime error: Unsupported hash request.");

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
			case OPERATION_TYPE_PROC: break;
			case OPERATION_TYPE_STRUCT: break;
			case OPERATION_TYPE_O_LIST: break;
			case OPERATION_TYPE_LIST: break;
			case OPERATION_TYPE_INDEX: break;
			case OPERATION_TYPE_EXEC: break;
			case OPERATION_TYPE_TO_NUM: break;
			case OPERATION_TYPE_TO_BOOL: break;
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
		}
	}
	
	return ret;
}

bool_t operation_equ(operation_t* o1, operation_t* o2) {

	error("Runtime error: Unsupported equ request.");

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
		case OPERATION_TYPE_PROC: break;
		case OPERATION_TYPE_STRUCT: break;
		case OPERATION_TYPE_O_LIST: break;
		case OPERATION_TYPE_LIST: break;
		case OPERATION_TYPE_INDEX: break;
		case OPERATION_TYPE_EXEC: break;
		case OPERATION_TYPE_TO_NUM: break;
		case OPERATION_TYPE_TO_BOOL: break;
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
	}

	return ret;
}