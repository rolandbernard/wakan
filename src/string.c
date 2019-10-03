// Copyright (c) 2018-2019 Roland Bernard

#include <stdlib.h>
#include <math.h>

#include "./prime.h"
#include "./string.h"
#include "./langallocator.h"

string_t* string_create(const char* str) {
    size_t length;

    length = 0;
    if(str != NULL) {
        while(str[length])
            length++;
    }

    return string_create_full(str, length);
}

string_t* string_create_full(const char* str, size_t length) {
    string_t* ret = (string_t*)_alloc(sizeof(string_t));

    ret->data = (char*)_alloc((sizeof(char)*length + 1));
    ret->length = length;
    while(length) {
        length--;
        ret->data[length] = str[length];
    }
    ret->data[ret->length] = 0;

    return ret;
}

string_t* string_copy(string_t* str) {
    string_t* ret = NULL;

    if(str != NULL) {
        ret = (string_t*)_alloc(sizeof(string_t));
        ret->data = (char*)_alloc(sizeof(char)*(str->length + 1));
        for(ret->length = 0; ret->length < str->length; ret->length++)
            ret->data[ret->length] = str->data[ret->length];
        ret->data[ret->length] = 0;
    }

    return ret;
}

string_t* string_concat(string_t* s1, string_t* s2) {
    string_t* ret = NULL;
    
    if(s1 != NULL && s2 != NULL) {
        ret = (string_t*)_alloc(sizeof(string_t));
        ret->data = (char*)_alloc(sizeof(char)*(s1->length + s2->length + 1));
        ret->length = 0;
        while(ret->length < s1->length) {
            ret->data[ret->length] = s1->data[ret->length];
            ret->length++;
        }

        while(ret->length < s1->length + s2->length) {
            ret->data[ret->length] = s2->data[ret->length - s1->length];
            ret->length++;
        }
        ret->data[ret->length] = 0;
    }

    return ret;
}

string_t* string_concat_and_free(string_t* s1, string_t* s2) {
    string_t* ret = string_concat(s1, s2);
    string_free(s1);
    string_free(s2);
    return ret;
}

string_t* string_mult(string_t* str, size_t n) {
    string_t* ret = NULL;
    
    if(str != NULL) {
        ret = (string_t*)_alloc(sizeof(string_t));
        ret->data = (char*)_alloc(sizeof(char)*(str->length * n + 1));
        ret->length = 0;
        while(ret->length < str->length * n) {
            ret->data[ret->length] = str->data[ret->length % str->length];
            ret->length++;
        }
        ret->data[ret->length] = 0;
    }

    return ret;
}

string_t* string_substr(string_t* str, pos_t pos, pos_t n) {
    string_t* ret = NULL;

    if(n >= 0) {
        if(str != NULL && str->length >= pos + n && pos >= 0) {
            ret = (string_t*)_alloc(sizeof(string_t));
            ret->data = (char*)_alloc(sizeof(char)*(n + 1));
            ret->length = n;
            while(n--) {
                ret->data[n] = str->data[pos+n];
            }
            ret->data[ret->length] = 0;
        }
    } else {
        if(str != NULL && 0 < pos + n && pos < str->length) {
            ret = (string_t*)_alloc(sizeof(string_t));
            ret->data = (char*)_alloc(sizeof(char)*((-n) + 1));
            ret->length = -n;
            while(n++) {
                ret->data[-n] = str->data[pos+n];
            }
            ret->data[ret->length] = 0;
        }
    }

    return ret;
}

pos_t string_find(string_t* str, string_t* find) {
    pos_t ret = -1;

    if(str != NULL) {
        int i;
        for(i = 0; ret == -1 && i <= (int)str->length - (int)find->length; i++) {
            int j = 0;

            while(j < find->length) {
                if(str->data[i + j] != find->data[j])
                    break;
                j++;
            }

            if(j == find->length)
                ret = i;
        }
    }

    return ret;
}

pos_t string_find_from(string_t* str, string_t* find, pos_t pos) {
    pos_t ret = -1;

    if(str != NULL) {
        int i;
        for(i = pos; ret == -1 && i <= (int)str->length - (int)find->length; i++) {
            int j = 0;

            while(j < find->length) {
                if(str->data[i + j] != find->data[j])
                    break;
                j++;
            }

            if(j == find->length)
                ret = i;
        }
    }

    return ret;
}

size_t string_length(string_t* str) {
    if(str != NULL)
        return str->length;
    else 
        return 0;
}

// TODO:
id_t string_id(string_t* str) {
    long hash = 0;
    for (int i = 0; i < str->length; i++) {
        hash += (long)pow(SMALL_PRIME_1, str->length - (i+1)) * str->data[i];
    }
    return (id_t)hash;
}

bool_t string_equ(string_t* s1, string_t* s2) {
    if(s1 != NULL && s2 != NULL) {
        if(s1->length != s2->length)
            return false;
        int i;
        for(i = 0; i < s1->length; i++)
            if(s1->data[i] != s2->data[i])
                return false;
        return true;
    } else 
        return s1 == s2;
}

int string_cmp(string_t* s1, string_t* s2) {
    if(s1 != NULL && s2 != NULL) {
        int i;
        for(i = 0; i < s1->length && i < s2->length; i++)
            if(s1->data[i] != s2->data[i])
                return (s1->data[i] > s2->data[i] ? 1 : -1);
        if(s1->length != s2->length)
                return (s1->length > s2->length ? 1 : -1);
        return 0;
    } else 
        return s1 != s2;
}

char string_char_at(string_t* str, pos_t pos) {
    if(str != NULL && pos >= 0 && pos < str->length)
        return str->data[pos];
    else
        return 0xFF;
}

void string_free(string_t* str) {
    if(str != NULL) {
        if(str->data != NULL)
            _free(str->data);
        str->length = 0;
        _free(str);
    }
}

char* string_get_cstr(string_t* str) {
    if(str != NULL)
        return str->data;
    else 
        return NULL;
}