// Copyright (c) 2018-2019 Roland Bernard

#include "./token.h"
#include "./langallocator.h"

token_t* token_create() {
    return (token_t*)_alloc(sizeof(token_t));
}
