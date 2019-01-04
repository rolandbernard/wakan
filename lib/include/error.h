// Copyright (c) 2018-2019 Roland Bernard

#ifndef __ERROR_H__
#define __ERROR_H__

#include "./types.h"
#include "./bool.h"

typedef void (*error_handler_t)(const char*);

void error(const char* msg);
void set_error_handler(error_handler_t handler);
error_handler_t get_error_handler();
void default_error_handler(const char* msg);
void set_stack_start(void* start);
bool_t check_for_stackoverflow();


#endif