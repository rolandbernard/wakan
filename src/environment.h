#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include "./types.h"
#include "./string.h"
#include "./variabletable.h"

typedef struct environment_s {
	variabletable_t** data;
	size_t size;
	size_t count;
	size_t local_mode_limit;
} environment_t;

environment_t* environment_create();
void environment_write(environment_t* env, string_t* name, object_t* obj);
object_t* environment_get(environment_t* env, string_t* name);
object_t** environment_get_var(environment_t* env, string_t* name);
void environment_make(environment_t* env, string_t* name);
void environment_del(environment_t* env, string_t* name);
void environment_add_scope(environment_t* env);
void environment_remove_scope(environment_t* env);
void environment_free(environment_t* env);
id_t environment_id(environment_t* env);
bool_t environment_equ(environment_t* e1, environment_t* e2);
void environment_set_local_mode(environment_t* env, size_t local_mode_limit);

#endif