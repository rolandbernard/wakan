// Copyright (c) 2018 Roland Bernard

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "./program.h"
#include "./environment.h"
#include "./error.h"
#include "./langallocator.h"

int main(int argc, char** argv) {
	set_stack_start(&argc);
	srand(time(NULL) + clock());
	environment_t* env = environment_create();

	fseek(stdin, 0, SEEK_END);
	size_t file_size = ftell(stdin);
	fseek(stdin, 0, SEEK_SET);

	char* buffer = (char*)_alloc(sizeof(char)*(file_size+1));
	buffer[file_size] = '\0';
	fread(buffer, 1, file_size, stdin);

	program_t* program = tokenize_and_parse_program(buffer);
	program_exec(program, env);
	program_free(program);

	_free(buffer);

	for(int i = 1; i < argc; i++) {
		FILE* file = fopen(argv[i], "r");
		if(file == NULL) {
			printf("Couldn't open the file \"%s\".\n", argv[i]);
			exit(1);
		}

		fseek(file, 0, SEEK_END);
		file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

	    buffer = (char*)_alloc(sizeof(char)*(file_size+1));
		buffer[file_size] = '\0';
		fread(buffer, 1, file_size, file);

		program = tokenize_and_parse_program(buffer);
		program_exec(program, env);
		program_free(program);

		_free(buffer);
		fclose(file);
	}
}