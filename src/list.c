// Copyright (c) 2018-2019 Roland Bernard

#include <math.h>

#include "./list.h"
#include "./object.h"
#include "./langallocator.h"
#include "./prime.h"

list_t* list_create_empty() {
	list_t* ret = (list_t*)_alloc(sizeof(list_t));

	ret->data = NULL;
	ret->size = 0;

	return ret;
}

list_t* list_create_null(size_t size) {
	list_t* ret = (list_t*)_alloc(sizeof(list_t));

	ret->data = (object_t**)_alloc(size * sizeof(object_t*));
	ret->size = 0;
	while(ret->size < size) {
		ret->data[ret->size] = NULL;
		ret->size++;
	}

	return ret;
}

list_t* list_copy(list_t* list) {
	list_t* ret = NULL;

	if(list != NULL) {
		ret = (list_t*)_alloc(sizeof(list_t));
		ret->data = (object_t**)_alloc(sizeof(object_t*) * list->size);
		for(ret->size = 0; ret->size < list->size; ret->size++) {
			ret->data[ret->size] = list->data[ret->size];
			object_reference(ret->data[ret->size]);
		}
	}

	return ret;
}

list_t* list_add(list_t* l1, list_t* l2) {
	list_t* ret = NULL;
	
	if(l1 != NULL && l2 != NULL) {
		ret = (list_t*)_alloc(sizeof(list_t));
		ret->data = (object_t**)_alloc((l1->size + l2->size) * sizeof(object_t*));
		ret->size = 0;
		while(ret->size < l1->size) {
			ret->data[ret->size] = l1->data[ret->size];
			object_reference(l1->data[ret->size]);
			ret->size++;
		}
		while(ret->size < l1->size + l2->size) {
			ret->data[ret->size] = l2->data[ret->size - l1->size];
			object_reference(l2->data[ret->size - l1->size]);
			ret->size++;
		}
	}

	return ret;
}

list_t* list_mul(list_t* list, size_t n) {
	list_t* ret = NULL;
	
	if(list != NULL) {
		ret = (list_t*)_alloc(sizeof(list_t));
		ret->data = (object_t**)_alloc(list->size * n * sizeof(object_t*));
		ret->size = 0;
		while(ret->size < list->size * n) {
			ret->data[ret->size] = list->data[ret->size % list->size];
			object_reference(ret->data[ret->size]);
			ret->size++;
		}
	}

	return ret;
}

list_t* list_range(list_t* list, pos_t pos, pos_t n) {
	list_t* ret = NULL;

	if(n >= 0) {
		if(list != NULL && list->size >= pos+n && pos >= 0) {
			ret = (list_t*)_alloc(sizeof(list_t));
			ret->data = (object_t**)_alloc(n * sizeof(object_t*));
			ret->size = n;
			while(n--) {
				ret->data[n] = list->data[pos + n];
				object_reference(ret->data[n]);
			}
		}
	} else {
		if(list != NULL && 0 <= pos+n+1 && pos < list->size) {
			ret = (list_t*)_alloc(sizeof(list_t));
			ret->data = (object_t**)_alloc((-n) * sizeof(object_t*));
			ret->size = -n;
			while(n++) {
				ret->data[-n] = list->data[pos + n];
				object_reference(ret->data[-n]);
			}
		}
	}

	return ret;
}

pos_t list_find(list_t* list, object_t* obj) {
	pos_t ret = -1;

	if(list != NULL) {
		int i;
		for(i = 0; i < list->size && ret == -1; i++)
			if(object_equ(list->data[i], obj))
				ret = i;
	}
	return ret;
}

object_t* list_get(list_t* list, pos_t pos) {
	if(list != NULL) {
		if(pos >= list->size || pos < 0)
			return NULL;
		else
			return list->data[pos];
	} else 
		return NULL;
}

object_t** list_get_loc(list_t* list, pos_t pos) {
	if(list != NULL) {
		if(pos >= list->size || pos < 0)
			return NULL;
		else
			return &(list->data[pos]);
	} else 
		return NULL;
}

size_t list_size(list_t* list) {
	if(list != NULL)
		return list->size;
	else
		return 0; 
}

id_t list_id(list_t* list) {
	long hash = 0;
    for (int i = 0; i < list->size; i++) {
        hash += (long)pow(BIG_PRIME_3, list->size - (i+1)) * object_id(list->data[i]);
    }
    return (id_t)hash;
}

bool_t list_equ(list_t* l1, list_t* l2) {
	if(l1 != NULL && l2 != NULL) {
		if(l1->size != l2->size)
			return false;
		int i;
		for(i = 0; i < l1->size; i++)
			if(!object_equ(l1->data[i], l2->data[i]))
				return false;
		return true;
	} else
		return l1 == l2;
}

void list_append(list_t* list, object_t* obj) {
	if(list != NULL) {
		int i;
		object_t** tmp = (object_t**)_alloc((list->size + 1) * sizeof(object_t*));
		for(i = 0; i < list->size; i++)
			tmp[i] = list->data[i];
		tmp[i] = obj;
		object_reference(obj);
		if(list->data != NULL)
			_free(list->data);
		list->data = tmp;
		list->size++;
	}
}

void list_free(list_t* list) {
	if(list != NULL) {
		for(int i = 0; i < list->size; i++)
			object_dereference(list->data[i]);
		if(list->data != NULL)
			_free(list->data);
		list->size = 0;
		_free(list);
	}
}