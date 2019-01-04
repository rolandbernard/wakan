// Copyright (c) 2018-2019 Roland Bernard

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "./types.h"
#include "./number.h"
#include "./bool.h"
#include "./string.h"
#include "./list.h"
#include "./pair.h"
#include "./dictionary.h"
#include "./function.h"
#include "./macro.h"
#include "./struct.h"

extern bool_t empty_line;

typedef enum object_type_e {
	OBJECT_TYPE_NONE,
	OBJECT_TYPE_NUMBER,
	OBJECT_TYPE_BOOL,
	OBJECT_TYPE_STRING,
	OBJECT_TYPE_PAIR,
	OBJECT_TYPE_LIST,
	OBJECT_TYPE_DICTIONARY,
	OBJECT_TYPE_FUNCTION,
	OBJECT_TYPE_MACRO,
	OBJECT_TYPE_STRUCT,
	/*...*/
} object_type_t;

typedef struct object_s {
	size_t num_references;
	object_type_t type;
	union
	{
		number_t number;
		bool_t boolean;
		string_t* string;
		pair_t* pair;
		list_t* list;
		dictionary_t* dic;
		function_t* func;
		macro_t* mac;
		struct_t* stc;
		/*...*/
	} data;

} object_t;


object_t* object_create_none();
object_t* object_create_number(number_t number);
object_t* object_create_boolean(bool_t boolean);
object_t* object_create_string(string_t* string);
object_t* object_create_pair(pair_t* pair);
object_t* object_create_list(list_t* list);
object_t* object_create_dictionary(dictionary_t* dic);
object_t* object_create_function(function_t* func);
object_t* object_create_macro(macro_t* mac);
object_t* object_create_struct(struct_t* stc);


id_t object_id(object_t* obj);
bool_t object_equ(object_t* o1, object_t* o2);
void object_reference(object_t* obj);
bool_t object_dereference(object_t* obj);
bool_t object_check_reference(object_t* obj);
void object_free(object_t* obj);
void print_object(object_t* obj);
string_t* object_to_string(object_t* obj);
bool_t is_true(object_t* obj);
object_t* object_add(object_t* o1, object_t* o2);
object_t* object_mul(object_t* o1, object_t* o2);

#endif
