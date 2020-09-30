// Copyright (c) 2018-2019 Roland Bernard

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    #include <windows.h> // The Windows code has not been implemented yet.
#else
    #include <sys/resource.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "./error.h"

static error_handler_t curr_handler = default_error_handler;

void error(const char* msg) {
    curr_handler(msg);
}

void set_error_handler(error_handler_t handler) {
    curr_handler = handler;
}

error_handler_t get_error_handler() {
    return curr_handler;
}

void default_error_handler(const char* msg) {
    fprintf(stderr, "Error: %s (aborting process).\n", msg);
    abort();
}

void set_stack_start(void* start) {
}

bool_t check_for_stackoverflow() {
    return false; // Now using split stack
}
