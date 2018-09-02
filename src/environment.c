// Copyright (c) 2018 Roland Bernard

#include <math.h>

#include "./prime.h"
#include "./environment.h"
#include "./object.h"
#include "./langallocator.h"


environment_t* environment_create() {
	environment_t* ret = (environment_t*)_alloc(sizeof(environment_t));
	
	ret->count = 1;
	ret->size = 1;
	ret->data = (variabletable_t**)_alloc(sizeof(variabletable_t*));
	ret->local_mode_limit = 0;

	ret->data[0] = variabletable_create();

	return ret;
}

void environment_write(environment_t* env, string_t* name, object_t* obj) {
	if(env != NULL) {
		int i = env->count-1;
		while(i >= (signed)env->local_mode_limit && !variabletable_exists(env->data[i], name)) i--;
		if(i < (signed)env->local_mode_limit) {
			variabletable_write(env->data[env->count-1], name, obj);
		} else {
			variabletable_write(env->data[i], name, obj);
		}
	}
}

object_t* environment_get(environment_t* env, string_t* name) {
	if(env != NULL) {
		int i = env->count-1;
		while(i >= (signed)env->local_mode_limit && !variabletable_exists(env->data[i], name)) i--;
		if(i < (signed)env->local_mode_limit) {
			return variabletable_get(env->data[env->count-1], name);
		} else {
			return variabletable_get(env->data[i], name);
		}
	} else 
		return NULL;
}

object_t** environment_get_var(environment_t* env, string_t* name) {
	if(env != NULL) {
		int i = env->count-1;
		while(i >= (signed)env->local_mode_limit && !variabletable_exists(env->data[i], name)) i--;
		if(i < (signed)env->local_mode_limit) {
			return variabletable_get_loc(env->data[env->count-1], name);
		} else {
			return variabletable_get_loc(env->data[i], name);
		}
	} else
		return NULL;
}

void environment_make(environment_t* env, string_t* name) {
	if(env != NULL) {
		int i = env->count-1;
		while(i >= (signed)env->local_mode_limit &&	!variabletable_exists(env->data[i], name)) i--;
		if(i < (signed)env->local_mode_limit) {
			variabletable_make(env->data[env->count-1], name);
		} else {
			variabletable_make(env->data[i], name);
		}
	}
}

void environment_del(environment_t* env, string_t* name) {
	if(env != NULL) {
		int i = env->count-1;
		while(i >= (signed)env->local_mode_limit && !variabletable_exists(env->data[i], name)) i--;
		if(i < (signed)env->local_mode_limit) {
			variabletable_del(env->data[env->count-1], name);
		} else {
			variabletable_del(env->data[i], name);
		}
	}
}

void environment_add_scope(environment_t* env) {
	if(env != NULL) {
		if(env->size == env->count) {
			variabletable_t** tmp = (variabletable_t**)_alloc(sizeof(variabletable_t*)*(env->size+1));
			for(int i = 0; i < env->size; i++)
				tmp[i] = env->data[i];
			_free(env->data);
			env->data = tmp;
			env->size++;
		}
		env->data[env->count] = variabletable_create();
		env->count++;
	}
}

void environment_remove_scope(environment_t* env) {
	if(env != NULL) {
		if(env->count > 1) {
			variabletable_free(env->data[env->count-1]);
			env->count--;
		}
	}
}

void environment_free(environment_t* env) {
	if(env != NULL) {
		for(int i = 0; i < env->count; i++)
			variabletable_free(env->data[i]);
		_free(env->data);
		_free(env);
	}
}

id_t environment_id(environment_t* env) {
	long hash = 0;
	for(int i = 0; i < env->count; i++)
		hash += variabletable_id(env->data[i])*(long)pow(BIG_PRIME_2, i);
	return (id_t)hash;
}

bool_t environment_equ(environment_t* e1, environment_t* e2) {
	if(e1->count != e2->count)
		return false;

	for(int i = 0; i < e1->count; i++)
		if(!variabletable_equ(e1->data[i], e2->data[i]))
			return false;

	return true;
}


void environment_set_local_mode(environment_t* env, size_t local_mode_limit) {
	env->local_mode_limit = local_mode_limit;
}