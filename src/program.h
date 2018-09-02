#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include "./tokenlist.h"
#include "./operation.h"
#include "./environment.h"

typedef	operation_t program_t;

program_t* parse_program(tokenlist_t* tokens);
program_t* tokenize_and_parse_program(const char* src);
void program_free(program_t* program);
void program_exec(program_t* program, environment_t* env);

#endif