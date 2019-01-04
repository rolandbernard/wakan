// Copyright (c) 2018-2019 Roland Bernard

#ifndef __NUMBER_H__
#define __NUMBER_H__

#include "./types.h"
#include "./bool.h"

// TODO: Upgrade number_t
typedef double number_t;

id_t number_id(number_t num);
int number_cmp(number_t n1, number_t n2);
bool_t number_equ(number_t n1, number_t n2);

#endif