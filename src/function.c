
#include "./function.h"
#include "./environment.h"
#include "./object.h"
#include "./prime.h"
#include "./langallocator.h"


function_t* function_create(operation_t* par, operation_t* func) {
	function_t* ret = (function_t*)_alloc(sizeof(function_t));

	ret->parameter = par;
	ret->function = func;

	return ret;
}

void function_exec(function_t* func, object_t** par, environment_t* env) {
	if(func != NULL) {
		environment_add_scope(env);
		size_t prev_limit = env->local_mode_limit;
		environment_set_local_mode(env, env->count-1);

		object_t*** par_loc_list = operation_var(func->parameter, env);

		if(par != NULL && par_loc_list != NULL)
			for(int i = 0; par[i] != NULL && par_loc_list[i] != NULL; i++) {
				object_dereference(*(par_loc_list[i]));
				*(par_loc_list[i]) = par[i];
				object_reference(*(par_loc_list[i]));
			}

		operation_exec(func->function, env);
		
		environment_set_local_mode(env, prev_limit);
		environment_remove_scope(env);
	}
}


object_t** function_result(function_t* func, object_t** par, environment_t* env) {
	object_t** ret = NULL;

	if(func != NULL) {
		environment_add_scope(env);
		size_t prev_limit = env->local_mode_limit;
		environment_set_local_mode(env, env->count-1);

		object_t*** par_loc_list = operation_var(func->parameter, env);

		if(par != NULL && par_loc_list != NULL)
			for(int i = 0; par[i] != NULL && par_loc_list[i] != NULL; i++) {
				object_dereference(*(par_loc_list[i]));
				*(par_loc_list[i]) = par[i];
				object_reference(*(par_loc_list[i]));
			}

		ret = operation_result(func->function, env);
		
		environment_set_local_mode(env, prev_limit);
		environment_remove_scope(env);
	}

	return ret;
}

void function_free(function_t* func) {
	if(func != NULL) {
		// Operations in Objects don't have to be freed since they are part of the tree
		//operation_free(func->function);
		//operation_free(func->parameter);
		_free(func);
	}
}

id_t function_id(function_t* func) {
	return SMALL_PRIME_2 * operation_id(func->function) + operation_id(func->parameter);
}

bool_t function_equ(function_t* f1, function_t* f2) {
	return operation_equ(f1->function, f2->function) && operation_equ(f1->parameter, f2->parameter);
}