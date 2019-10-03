// Copyright (c) 2018-2019 Roland Bernard

#ifndef __STRING_H__
#define __STRING_H__

#include "./types.h"
#include "./bool.h"

typedef struct string_s {
    char* data;
    size_t length;
} string_t;

string_t* string_create(const char* str);
string_t* string_create_full(const char* str, size_t length);
string_t* string_copy(string_t* str);
string_t* string_concat(string_t* s1, string_t* s2);
string_t* string_concat_and_free(string_t* s1, string_t* s2);
string_t* string_mult(string_t* str, size_t n);
string_t* string_substr(string_t* str, pos_t pos, pos_t n);
pos_t string_find(string_t* str, string_t* find);
pos_t string_find_from(string_t* str, string_t* find, pos_t pos);
size_t string_length(string_t* str);
id_t string_id(string_t* str);
bool_t string_equ(string_t* s1, string_t* s2);
int string_cmp(string_t* s1, string_t* s2);
char string_char_at(string_t* str, pos_t pos);
void string_free(string_t* str);
char* string_get_cstr(string_t* str);

#endif