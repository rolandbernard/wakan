// Copyright (c) 2018 Roland Bernard

#include <stdio.h>
#include <math.h>

#include "./object.h"
#include "./langallocator.h"
#include "./prime.h"
#include "./error.h"

#define TMP_STR_MAX 1<<12

object_t* object_create_none() {
	object_t* ret = (object_t*)_alloc(sizeof(object_t));
	ret->num_references = 0;
	ret->type = OBJECT_TYPE_NONE;
	return ret;
}

object_t* object_create_number(number_t number) {
	object_t* ret = (object_t*)_alloc(sizeof(object_t));
	ret->num_references = 0;
	ret->type = OBJECT_TYPE_NUMBER;
	ret->data.number = number;
	return ret;
}
 
object_t* object_create_boolean(bool_t boolean) {
	object_t* ret = (object_t*)_alloc(sizeof(object_t));
	ret->num_references = 0;
	ret->type = OBJECT_TYPE_BOOL;
	ret->data.boolean = boolean;
	return ret;
}

object_t* object_create_string(string_t* string) {
	object_t* ret = (object_t*)_alloc(sizeof(object_t));
	ret->num_references = 0;
	ret->type = OBJECT_TYPE_STRING;
	ret->data.string = string;
	return ret;
}

object_t* object_create_pair(pair_t* pair) {
	object_t* ret = (object_t*)_alloc(sizeof(object_t));
	ret->num_references = 0;
	ret->type = OBJECT_TYPE_PAIR;
	ret->data.pair = pair;
	return ret;
}

object_t* object_create_list(list_t* list) {
	object_t* ret = (object_t*)_alloc(sizeof(object_t));
	ret->num_references = 0;
	ret->type = OBJECT_TYPE_LIST;
	ret->data.list = list;
	return ret;
}

object_t* object_create_dictionary(dictionary_t* dic) {
	object_t* ret = (object_t*)_alloc(sizeof(object_t));
	ret->num_references = 0;
	ret->type = OBJECT_TYPE_DICTIONARY;
	ret->data.dic = dic;
	return ret;
}

object_t* object_create_function(function_t* func) {
	object_t* ret = (object_t*)_alloc(sizeof(object_t));
	ret->num_references = 0;
	ret->type = OBJECT_TYPE_FUNCTION;
	ret->data.func = func;
	return ret;
}

object_t* object_create_macro(macro_t* mac) {
	object_t* ret = (object_t*)_alloc(sizeof(object_t));
	ret->num_references = 0;
	ret->type = OBJECT_TYPE_MACRO;
	ret->data.mac = mac;
	return ret;
}

object_t* object_create_struct(struct_t* stc) {
	object_t* ret = (object_t*)_alloc(sizeof(object_t));
	ret->num_references = 0;
	ret->type = OBJECT_TYPE_STRUCT;
	ret->data.stc = stc;
	return ret;
}


// TODO: hash for function, macro and struct
id_t object_id(object_t* obj) {
	if(obj == NULL)
		return HUGE_PRIME_5;
	switch(obj->type)
	{
		case OBJECT_TYPE_NONE: return HUGE_PRIME_4; break;
		case OBJECT_TYPE_NUMBER: return number_id(obj->data.number); break;
		case OBJECT_TYPE_BOOL: return bool_id(obj->data.boolean); break;
		case OBJECT_TYPE_STRING: return string_id(obj->data.string); break;
		case OBJECT_TYPE_PAIR: return pair_id(obj->data.pair); break;
		case OBJECT_TYPE_LIST: return list_id(obj->data.list); break;
		case OBJECT_TYPE_DICTIONARY: return dictionary_id(obj->data.dic); break;
		case OBJECT_TYPE_FUNCTION: return function_id(obj->data.func); break;
		case OBJECT_TYPE_MACRO: return macro_id(obj->data.mac); break;
		case OBJECT_TYPE_STRUCT: return struct_id(obj->data.stc); break;
	}
	return 0;
}

// TODO: equ for function, macro and struct
bool_t object_equ(object_t* o1, object_t* o2) {
	if(o1 == NULL && o2 == NULL)
		return true;
	else if(o1 == NULL || o2 == NULL)
		return false;
	else if(o1->type != o2->type)
		return false;

	switch(o1->type)
	{
		case OBJECT_TYPE_NONE: return 1; break;
		case OBJECT_TYPE_NUMBER: return number_equ(o1->data.number, o2->data.number); break;
		case OBJECT_TYPE_BOOL: return bool_equ(o1->data.boolean, o2->data.boolean); break;
		case OBJECT_TYPE_STRING: return string_equ(o1->data.string, o2->data.string); break;
		case OBJECT_TYPE_PAIR: return pair_equ(o1->data.pair, o2->data.pair); break;
		case OBJECT_TYPE_LIST: return list_equ(o1->data.list, o2->data.list); break;
		case OBJECT_TYPE_DICTIONARY: return dictionary_equ(o1->data.dic, o2->data.dic); break;
		case OBJECT_TYPE_FUNCTION: return function_equ(o1->data.func, o2->data.func); break;
		case OBJECT_TYPE_MACRO: return macro_equ(o1->data.mac, o2->data.mac); break;
		case OBJECT_TYPE_STRUCT: return struct_equ(o1->data.stc, o2->data.stc); break;
	}
	return false;
}

void object_reference(object_t* obj) {
	if(obj != NULL && obj != OBJECT_LIST_OPENED)
		obj->num_references++;
}

bool_t object_dereference(object_t* obj) {
	if(obj != NULL && obj != OBJECT_LIST_OPENED && obj->num_references > 0 )
		obj->num_references--;
	return object_check_reference(obj);
}

bool_t object_check_reference(object_t* obj) {
	if(obj != NULL && obj != OBJECT_LIST_OPENED && obj->num_references == 0) {
		object_free(obj);
		return true;
	}
	return false;
}

// TODO:
void object_free(object_t* obj) {
	if(obj != NULL && obj != OBJECT_LIST_OPENED) {
		switch(obj->type)
		{
			case OBJECT_TYPE_NONE: break;
			case OBJECT_TYPE_NUMBER: break;
			case OBJECT_TYPE_BOOL: break;
			case OBJECT_TYPE_STRING: string_free(obj->data.string); break;
			case OBJECT_TYPE_PAIR: pair_free(obj->data.pair); break;
			case OBJECT_TYPE_LIST: list_free(obj->data.list); break;
			case OBJECT_TYPE_DICTIONARY: dictionary_free(obj->data.dic); break;
			case OBJECT_TYPE_FUNCTION: function_free(obj->data.func); break;
			case OBJECT_TYPE_MACRO: macro_free(obj->data.mac); break;
			case OBJECT_TYPE_STRUCT: struct_free(obj->data.stc); break;
		}
		_free(obj);
	}
}

void print_object(object_t* obj) {
	if(obj == NULL || obj == OBJECT_LIST_OPENED)
		fprintf(stdout, "null");
	else {
		switch(obj->type) {
			case OBJECT_TYPE_NONE: fprintf(stdout, "none"); break;
			case OBJECT_TYPE_NUMBER: fprintf(stdout, "%.15lg", obj->data.number); break;
			case OBJECT_TYPE_BOOL: fprintf(stdout, (obj->data.boolean ? "true" : "false")); break;
			case OBJECT_TYPE_STRING: fprintf(stdout, "%s", string_get_cstr(obj->data.string)); break;
			case OBJECT_TYPE_PAIR: 
				if(obj->data.pair->key->type == OBJECT_TYPE_STRING)
					fprintf(stdout, "\"");
				print_object(obj->data.pair->key);
				if(obj->data.pair->key->type == OBJECT_TYPE_STRING)
					fprintf(stdout, "\"");
				fprintf(stdout, ":");
				if(obj->data.pair->value->type == OBJECT_TYPE_STRING)
					fprintf(stdout, "\"");
				print_object(obj->data.pair->value);
				if(obj->data.pair->value->type == OBJECT_TYPE_STRING)
					fprintf(stdout, "\"");
			break;
			case OBJECT_TYPE_LIST: 
				fprintf(stdout, "[");
				for(int i = 0; i < obj->data.list->size; i++) {
					if(obj->data.list->data[i]->type == OBJECT_TYPE_STRING)
						fprintf(stdout, "\"");
					print_object(obj->data.list->data[i]);
					if(obj->data.list->data[i]->type == OBJECT_TYPE_STRING)
						fprintf(stdout, "\"");
					fprintf(stdout, ",");
				}
				fprintf(stdout, "]");
			break;
			case OBJECT_TYPE_DICTIONARY: 
				fprintf(stdout, "dic(");
				for(int i = 0; i < obj->data.dic->size; i++) 
					if(obj->data.dic->data[i] != NULL) {
						if(obj->data.dic->data[i]->key->type == OBJECT_TYPE_STRING)
							fprintf(stdout, "\"");
						print_object(obj->data.dic->data[i]->key);
						if(obj->data.dic->data[i]->key->type == OBJECT_TYPE_STRING)
							fprintf(stdout, "\"");
						fprintf(stdout, ":");
						if(obj->data.dic->data[i]->value->type == OBJECT_TYPE_STRING)
							fprintf(stdout, "\"");
						print_object(obj->data.dic->data[i]->value);
						if(obj->data.dic->data[i]->value->type == OBJECT_TYPE_STRING)
							fprintf(stdout, "\"");
						fprintf(stdout, ",");
					}
				fprintf(stdout, ")");
			break;
			case OBJECT_TYPE_FUNCTION: fprintf(stdout, "/function/"); break;
			case OBJECT_TYPE_MACRO: fprintf(stdout, "/macro/"); break;
			case OBJECT_TYPE_STRUCT: fprintf(stdout, "/struct/"); break;
		}
	}
}

string_t* object_to_string(object_t* obj) {
	string_t* ret = NULL;
	if(obj == NULL || obj == OBJECT_LIST_OPENED)
		ret = string_create("null");
	switch (obj->type) {
		case OBJECT_TYPE_NONE: ret = string_create("none"); break;
		case OBJECT_TYPE_NUMBER: {
			char temp_str[TMP_STR_MAX];
			sprintf(temp_str, "%.15g", obj->data.number);
			ret = string_create(temp_str);
		} break;
		case OBJECT_TYPE_BOOL: ret = string_create(obj->data.boolean ? "true" : "false"); break;
		case OBJECT_TYPE_STRING: ret = obj->data.string; break;
		case OBJECT_TYPE_LIST: 
			ret = string_create("");
			ret = string_concat_and_free(ret, string_create("["));
			for(int i = 0; i < obj->data.list->size; i++) {
				ret = string_concat_and_free(ret, object_to_string(obj->data.list->data[i]));
				ret = string_concat_and_free(ret, string_create(","));
			}
			ret = string_concat_and_free(ret, string_create("]"));
		break;
		case OBJECT_TYPE_PAIR: 
			ret = string_create("");
			ret = string_concat_and_free(ret, object_to_string(obj->data.pair->key));
			ret = string_concat_and_free(ret, string_create(":"));
			ret = string_concat_and_free(ret, object_to_string(obj->data.pair->value));
		break;
		case OBJECT_TYPE_DICTIONARY:
			ret = string_create("");
			ret = string_concat_and_free(ret, string_create("dic("));
			for(int i = 0; i < obj->data.dic->size; i++) 
				if(obj->data.dic->data[i] != NULL) {
					ret = string_concat_and_free(ret, object_to_string(obj->data.dic->data[i]->key));
					ret = string_concat_and_free(ret, string_create(":"));
					ret = string_concat_and_free(ret, object_to_string(obj->data.dic->data[i]->value));
					ret = string_concat_and_free(ret, string_create(","));
				}
			ret = string_concat_and_free(ret, string_create(")"));
		break;
		case OBJECT_TYPE_FUNCTION: ret = string_create("/function/"); break;
		case OBJECT_TYPE_MACRO: ret = string_create("/macro/"); break;
		case OBJECT_TYPE_STRUCT: ret = string_create("/struct/"); break;
	}
	return ret;
}

bool_t is_true(object_t* obj) {
	if(obj == NULL || obj == OBJECT_LIST_OPENED)
		return false;
	switch (obj->type) {
		case OBJECT_TYPE_NONE: return false; break;
		case OBJECT_TYPE_NUMBER: return obj->data.number == 0 ? false : true; break;
		case OBJECT_TYPE_BOOL: return obj->data.boolean; break;
		case OBJECT_TYPE_STRING: return string_length(obj->data.string) == 0 ? false : true; break;
		case OBJECT_TYPE_LIST: return list_size(obj->data.list) == 0 ? false : true; break;
		case OBJECT_TYPE_PAIR:
		case OBJECT_TYPE_DICTIONARY:
		case OBJECT_TYPE_FUNCTION:
		case OBJECT_TYPE_MACRO:
		case OBJECT_TYPE_STRUCT: return true; break;
	}
	return false;
}

object_t* object_add(object_t* o1, object_t* o2) {
	object_t* ret = NULL;
	if(o1->type == OBJECT_TYPE_NUMBER && o2->type == OBJECT_TYPE_NUMBER) {
		ret = object_create_number(o1->data.number + o2->data.number);
	} else if (o1->type == OBJECT_TYPE_LIST && o2->type == OBJECT_TYPE_LIST) {
		ret = object_create_list(list_add(o1->data.list, o2->data.list));
	} else if (o1->type == OBJECT_TYPE_STRING && o2->type == OBJECT_TYPE_STRING) {
		ret = object_create_string(string_concat(o1->data.string, o2->data.string));
	} else {
		error("Runtime error: Addition type error.");
		ret = RET_ERROR;
	}

	return ret;
}

object_t* object_mul(object_t* o1, object_t* o2) {
	object_t* ret = NULL;
	if(o1->type == OBJECT_TYPE_NUMBER && o2->type == OBJECT_TYPE_NUMBER) {
		ret = object_create_number(o1->data.number * o2->data.number);
	} else if (o1->type == OBJECT_TYPE_STRING && o2->type == OBJECT_TYPE_NUMBER) {
		ret = object_create_string(string_mult(o1->data.string, (size_t)round(o2->data.number)));
	} else if (o1->type == OBJECT_TYPE_NUMBER && o2->type == OBJECT_TYPE_STRING) {
		ret = object_create_string(string_mult(o2->data.string, (size_t)round(o1->data.number)));
	} else if (o1->type == OBJECT_TYPE_LIST && o2->type == OBJECT_TYPE_NUMBER) {
		ret = object_create_list(list_mul(o1->data.list, (size_t)round(o2->data.number)));
	} else if (o1->type == OBJECT_TYPE_NUMBER && o2->type == OBJECT_TYPE_LIST) {
		ret = object_create_list(list_mul(o2->data.list, (size_t)round(o1->data.number)));
	} else {
		error("Runtime error: Multiplication type error.");
		ret = RET_ERROR;
	}

	return ret;
}