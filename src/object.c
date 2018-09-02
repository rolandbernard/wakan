
#include "./object.h"
#include "./langallocator.h"
#include "./prime.h"

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
	if(obj != NULL)
		obj->num_references++;
}

bool_t object_dereference(object_t* obj) {
	if(obj != NULL && obj->num_references > 0)
		obj->num_references--;
	return object_check_reference(obj);
}

bool_t object_check_reference(object_t* obj) {
	if(obj != NULL && obj->num_references == 0) {
		object_free(obj);
		return true;
	}
	return false;
}

// TODO:
void object_free(object_t* obj) {
	if(obj != NULL) {
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
