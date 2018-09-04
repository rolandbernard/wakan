// Copyright (c) 2018 Roland Bernard

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	#include <windows.h> // The Windows code has not been implemented yet.
#else
	#include <sys/resource.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "./error.h"

static error_handler_t curr_handler = default_error_handler;
static void* stack_start = NULL;

void error(const char* msg) {
	curr_handler(msg);
}

void set_error_handler(error_handler_t handler) {
}

void default_error_handler(const char* msg) {
	fprintf(stderr, "Error: %s (aborting process).\n", msg);
	abort();
}

void set_stack_start(void* start) {
	stack_start = start;
}

bool_t check_for_stackoverflow() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	return false;
#else
	if(stack_start != NULL) {
		struct rlimit limit;
		getrlimit(RLIMIT_STACK, &limit);
		int top;
		if(stack_start - (void*)&top > limit.rlim_cur * 99 / 100) // We give ourself a 1% buffer
			return true;
		else 
			return false;
	} else
		return false;
#endif
}