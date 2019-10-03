// Copyright (c) 2018-2019 Roland Bernard

#ifndef __TOKENLIST_H__
#define __TOKENLIST_H__

#include "./token.h"
#include "./string.h"

typedef struct tokenlist_s {
    token_t* start;
    token_t* end;
} tokenlist_t;

tokenlist_t* tokenize(const char* src);
void tokenlist_free(tokenlist_t* list);

#endif