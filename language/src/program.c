
#include <stdarg.h>

#include "./program.h"
#include "./langallocator.h"
#include "./error.h"
#include "./types.h"

#define MAX_STACK_SIZE 1<<10

int get_operation_priority(operation_type_t type) {
	switch(type) {
		case OPERATION_TYPE_VAR:
		case OPERATION_TYPE_STR:
		case OPERATION_TYPE_NUM:
		case OPERATION_TYPE_BOOL:
		case OPERATION_TYPE_NONE:
		case OPERATION_TYPE_READ:
		case OPERATION_TYPE_RAND:
			return 0;
		case OPERATION_TYPE_EXEC:
		case OPERATION_TYPE_INDEX:
		case OPERATION_TYPE_IN_STRUCT:
			return 1;
		case OPERATION_TYPE_NOOP_BRAC:
		case OPERATION_TYPE_NOOP:
		case OPERATION_TYPE_LIST:
		case OPERATION_TYPE_NOOP_EMP_REC:
		case OPERATION_TYPE_SCOPE:
		case OPERATION_TYPE_NOOP_EMP_CUR: 
			return 2;
		case OPERATION_TYPE_FIND:
		case OPERATION_TYPE_SPLIT:
			return 3;
		case OPERATION_TYPE_IFELSE:
			return 4;
		case OPERATION_TYPE_IF:
		case OPERATION_TYPE_WHILE:
		case OPERATION_TYPE_FOR:
		case OPERATION_TYPE_FUNCTION:
		case OPERATION_TYPE_MACRO:
		case OPERATION_TYPE_STRUCT:
		case OPERATION_TYPE_DIC:
			return 5;
		case OPERATION_TYPE_SIN:
		case OPERATION_TYPE_COS:
		case OPERATION_TYPE_TAN:
		case OPERATION_TYPE_ASIN:
		case OPERATION_TYPE_ACOS:
		case OPERATION_TYPE_ATAN:
		case OPERATION_TYPE_SINH:
		case OPERATION_TYPE_COSH:
		case OPERATION_TYPE_TANH:
		case OPERATION_TYPE_ASINH:
		case OPERATION_TYPE_ACOSH:
		case OPERATION_TYPE_ATANH:
		case OPERATION_TYPE_TRUNC:
		case OPERATION_TYPE_FLOOR:
		case OPERATION_TYPE_CEIL:
		case OPERATION_TYPE_ROUND:
		case OPERATION_TYPE_LEN:
		case OPERATION_TYPE_CBRT:
		case OPERATION_TYPE_SQRT:
		case OPERATION_TYPE_TO_STR:
		case OPERATION_TYPE_TO_NUM:
		case OPERATION_TYPE_TO_BOOL:
		case OPERATION_TYPE_WRITE:
		case OPERATION_TYPE_LOCAL:
		case OPERATION_TYPE_GLOBAL:
		case OPERATION_TYPE_COPY:
			return 6;
		case OPERATION_TYPE_POW:
			return 7;
		case OPERATION_TYPE_MUL:
		case OPERATION_TYPE_DIV:
		case OPERATION_TYPE_MOD:
			return 8;
		case OPERATION_TYPE_ADD:
		case OPERATION_TYPE_SUB:
			return 9;
		case OPERATION_TYPE_NEG:
			return 8;
		case OPERATION_TYPE_ABS:
			return 9;
		case OPERATION_TYPE_EQU:
		case OPERATION_TYPE_GTR:
		case OPERATION_TYPE_LES:
		case OPERATION_TYPE_LEQ:
		case OPERATION_TYPE_GEQ:
			return 10;
		case OPERATION_TYPE_NOT:
			return 11;
		case OPERATION_TYPE_AND:
			return 12;
		case OPERATION_TYPE_OR:
		case OPERATION_TYPE_XOR:
			return 13;
		case OPERATION_TYPE_PAIR:
			return 14;
		case OPERATION_TYPE_O_LIST:
			return 15;
		case OPERATION_TYPE_NOOP_O_LIST_DEADEND:
			return 16;
		case OPERATION_TYPE_ASSIGN:
			return 17;
		case OPERATION_TYPE_PROC:
			return 18;
		case OPERATION_TYPE_NOOP_PROC_DEADEND:
			return 19;
		default: return 1024;
	}
}

bool_t is_on_stack(token_t** stack, size_t count, int n, ...) {
	bool_t ret = true;
	if(count < n)
		ret = false;
	else {
		va_list list;
		va_start(list, n);

		for(int i = n; ret && i > 0; i--) {
			if(stack[count-i]->type != va_arg(list, token_type_t))
				ret = false;
		}

		va_end(list);
	}
	return ret;
} 

// Assumes only one accordance
bool_t is_expected_on_stack(token_t** stack, size_t count, int n, ...) {
	bool_t ret = true;
	pos_t pos = -1;

	va_list list;
	va_start(list, n);
	token_type_t last = va_arg(list, token_type_t);
	for(int i = 0; pos == -1 && i < n-1; i++) {
		token_type_t tmp = va_arg(list, token_type_t);
		if(last == stack[count-1]->type && tmp == stack[count-1]->next->type)
			pos = i;
		else
			last = tmp;
	}
	va_end(list);

	if(pos == -1)
		ret = false;
	else {
		va_list list;
		va_start(list, n);	
		for(int i = pos; ret && i > 0; i--)
			if(va_arg(list, token_type_t) != stack[count-1-i]->type)
				ret = false;
		va_end(list);
	}

	return ret;
}

operation_type_t get_operation(token_t** stack, size_t count, bool_t(*func)(token_t** stack, size_t count, int n, ...)) {
	if(func(stack, count, 1, TOKEN_TYPE_VAR))
		return OPERATION_TYPE_VAR;
	else if(func(stack, count, 1, TOKEN_TYPE_STR))
		return OPERATION_TYPE_STR;
	else if(func(stack, count, 1, TOKEN_TYPE_NUM))
		return OPERATION_TYPE_NUM;
	else if(func(stack, count, 1, TOKEN_TYPE_BOOL))
		return OPERATION_TYPE_BOOL;
	else if(func(stack, count, 1, TOKEN_TYPE_NONE))
		return OPERATION_TYPE_NONE;
	else if(func(stack, count, 1, TOKEN_TYPE_READ))
		return OPERATION_TYPE_READ;
	else if(func(stack, count, 1, TOKEN_TYPE_RAND))
		return OPERATION_TYPE_RAND;
	else if(func(stack, count, 4, TOKEN_TYPE_EXP, TOKEN_TYPE_OPEN_BRAC, TOKEN_TYPE_EXP, TOKEN_TYPE_CLOSE_BRAC))
		return OPERATION_TYPE_EXEC;
	else if(func(stack, count, 4, TOKEN_TYPE_EXP, TOKEN_TYPE_OPEN_REC, TOKEN_TYPE_EXP, TOKEN_TYPE_CLOSE_REC))
		return OPERATION_TYPE_INDEX;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_DOT, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_IN_STRUCT;
	else if(func(stack, count, 3, TOKEN_TYPE_OPEN_BRAC, TOKEN_TYPE_EXP, TOKEN_TYPE_CLOSE_BRAC))
		return OPERATION_TYPE_NOOP_BRAC;
	else if(func(stack, count, 2, TOKEN_TYPE_OPEN_BRAC, TOKEN_TYPE_CLOSE_BRAC))
		return OPERATION_TYPE_NOOP;
	else if(func(stack, count, 3, TOKEN_TYPE_OPEN_REC, TOKEN_TYPE_EXP, TOKEN_TYPE_CLOSE_REC))
		return OPERATION_TYPE_LIST;
	else if(func(stack, count, 2, TOKEN_TYPE_OPEN_REC, TOKEN_TYPE_CLOSE_REC))
		return OPERATION_TYPE_NOOP_EMP_REC;
	else if(func(stack, count, 3, TOKEN_TYPE_OPEN_CUR, TOKEN_TYPE_EXP, TOKEN_TYPE_CLOSE_CUR))
		return OPERATION_TYPE_SCOPE;
	else if(func(stack, count, 2, TOKEN_TYPE_OPEN_CUR, TOKEN_TYPE_CLOSE_CUR))
		return OPERATION_TYPE_NOOP_EMP_CUR;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_FIND, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_FIND;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_SPLIT, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_SPLIT;
	else if(func(stack, count, 6, TOKEN_TYPE_IF, TOKEN_TYPE_EXP, TOKEN_TYPE_THEN, TOKEN_TYPE_EXP, TOKEN_TYPE_ELSE, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_IFELSE;
	else if(func(stack, count, 4, TOKEN_TYPE_IF, TOKEN_TYPE_EXP, TOKEN_TYPE_THEN, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_IF;
	else if(func(stack, count, 4, TOKEN_TYPE_WHILE, TOKEN_TYPE_EXP, TOKEN_TYPE_DO, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_WHILE;
	else if(func(stack, count, 8, TOKEN_TYPE_FOR, TOKEN_TYPE_EXP, TOKEN_TYPE_BACKSLASH, TOKEN_TYPE_EXP, TOKEN_TYPE_BACKSLASH, TOKEN_TYPE_EXP, TOKEN_TYPE_DO, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_FOR;
	else if(func(stack, count, 4, TOKEN_TYPE_FUNCTION, TOKEN_TYPE_EXP, TOKEN_TYPE_DOES, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_FUNCTION;
	else if(func(stack, count, 2, TOKEN_TYPE_DEF, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_MACRO;
	else if(func(stack, count, 2, TOKEN_TYPE_STRUCT, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_STRUCT;
	else if(func(stack, count, 2, TOKEN_TYPE_DIC, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_DIC;
	else if(func(stack, count, 2, TOKEN_TYPE_SIN, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_SIN;
	else if(func(stack, count, 2, TOKEN_TYPE_COS, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_COS;
	else if(func(stack, count, 2, TOKEN_TYPE_TAN, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_TAN;
	else if(func(stack, count, 2, TOKEN_TYPE_ASIN, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_ASIN;
	else if(func(stack, count, 2, TOKEN_TYPE_ACOS, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_ACOS;
	else if(func(stack, count, 2, TOKEN_TYPE_ATAN, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_ATAN;
	else if(func(stack, count, 2, TOKEN_TYPE_SINH, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_SINH;
	else if(func(stack, count, 2, TOKEN_TYPE_COSH, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_COSH;
	else if(func(stack, count, 2, TOKEN_TYPE_TANH, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_TANH;
	else if(func(stack, count, 2, TOKEN_TYPE_ASINH, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_ASINH;
	else if(func(stack, count, 2, TOKEN_TYPE_ACOSH, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_ACOSH;
	else if(func(stack, count, 2, TOKEN_TYPE_ATANH, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_ATANH;
	else if(func(stack, count, 2, TOKEN_TYPE_TRUNC, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_TRUNC;
	else if(func(stack, count, 2, TOKEN_TYPE_FLOOR, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_FLOOR;
	else if(func(stack, count, 2, TOKEN_TYPE_CEIL, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_CEIL;
	else if(func(stack, count, 2, TOKEN_TYPE_ROUND, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_ROUND;
	else if(func(stack, count, 2, TOKEN_TYPE_LEN, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_LEN;
	else if(func(stack, count, 2, TOKEN_TYPE_CBRT, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_CBRT;
	else if(func(stack, count, 2, TOKEN_TYPE_SQRT, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_SQRT;
	else if(func(stack, count, 2, TOKEN_TYPE_TO_STR, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_TO_STR;
	else if(func(stack, count, 2, TOKEN_TYPE_TO_NUM, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_TO_NUM;
	else if(func(stack, count, 2, TOKEN_TYPE_TO_BOOL, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_TO_BOOL;
	else if(func(stack, count, 2, TOKEN_TYPE_WRITE, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_WRITE;
	else if(func(stack, count, 2, TOKEN_TYPE_LOCAL, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_LOCAL;
	else if(func(stack, count, 2, TOKEN_TYPE_GLOBAL, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_GLOBAL;
	else if(func(stack, count, 2, TOKEN_TYPE_COPY, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_COPY;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_POW, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_POW;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_MUL, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_MUL;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_DIV, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_DIV;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_MOD, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_MOD;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_PLUS, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_ADD;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_MINUS, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_SUB;
	else if(func(stack, count, 2, TOKEN_TYPE_MINUS, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_NEG;
	else if(func(stack, count, 3, TOKEN_TYPE_ABS, TOKEN_TYPE_EXP, TOKEN_TYPE_ABS))
		return OPERATION_TYPE_ABS;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_EQU, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_EQU;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_GTR, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_GTR;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_LES, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_LES;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_GEQ, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_GEQ;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_LEQ, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_LEQ;
	else if(func(stack, count, 2, TOKEN_TYPE_NOT, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_NOT;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_AND, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_AND;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_OR, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_OR;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_XOR, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_XOR;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_PAIR, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_PAIR;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_ASSIGN, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_ASSIGN;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_COMMA, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_O_LIST;
	//else if(func(stack, count, 2, TOKEN_TYPE_EXP, TOKEN_TYPE_COMMA))
		//return OPERATION_TYPE_NOOP_O_LIST_DEADEND;
	else if(func(stack, count, 3, TOKEN_TYPE_EXP, TOKEN_TYPE_SEMICOL, TOKEN_TYPE_EXP))
		return OPERATION_TYPE_PROC;
	//else if(func(stack, count, 2, TOKEN_TYPE_EXP, TOKEN_TYPE_SEMICOL))
		//return OPERATION_TYPE_NOOP_PROC_DEADEND;
	else
		return ~0;
}

operation_type_t get_on_stack(token_t** stack, size_t count) {
	return get_operation(stack, count, is_on_stack);
}

operation_type_t get_expected_on_stack(token_t** stack, size_t count) {
	return get_operation(stack, count, is_expected_on_stack);
}

program_t* parse_program(tokenlist_t* list) {

	token_t** stack = (token_t**)_alloc(sizeof(token_t*)*MAX_STACK_SIZE);
	stack[0] = list->start;
	size_t count = 1;
	
	while(stack[count-1]->type != TOKEN_TYPE_END)
	{
		operation_type_t on_stack = get_on_stack(stack, count);
		if(on_stack == ~0) {
			// shift in next value
			stack[count] = stack[count-1]->next;
			count++;
		} else {
			// look ahead
			operation_type_t expected_on_stack = get_expected_on_stack(stack, count);
			if(on_stack == OPERATION_TYPE_POW ||
			  on_stack == OPERATION_TYPE_MUL ||
			  on_stack == OPERATION_TYPE_DIV || 
			  on_stack == OPERATION_TYPE_MOD ||
			  on_stack == OPERATION_TYPE_ADD ||
			  on_stack == OPERATION_TYPE_SUB ||
			  on_stack == OPERATION_TYPE_AND ||
			  on_stack == OPERATION_TYPE_OR ||
			  on_stack == OPERATION_TYPE_XOR ||
			  on_stack == OPERATION_TYPE_O_LIST ||
			  on_stack == OPERATION_TYPE_PROC) {
				  if(get_operation_priority(expected_on_stack) < get_operation_priority(on_stack) || on_stack == expected_on_stack) {
					// shift in next value
					stack[count] = stack[count-1]->next;
					count++;
				} else {
					// reduce
					token_t* tmp = token_create();
					tmp->type = TOKEN_TYPE_EXP;
					tmp->next = stack[count-1]->next;
					
					int num_of_repetitions = 0;
					while(get_on_stack(stack, count-num_of_repetitions*2) == on_stack) num_of_repetitions++;

					tmp->data.op = operation_create();
					tmp->data.op->type = on_stack;
					tmp->data.op->data.operations = (operation_t**)_alloc(sizeof(operation_t*)*(2+num_of_repetitions));
					tmp->data.op->data.operations[1+num_of_repetitions] = NULL;

					for(int i = 0; i < 1+num_of_repetitions; i++)
						tmp->data.op->data.operations[i] = stack[count - (1+num_of_repetitions*2) + (i*2)]->data.op;

					for(int i = 1; i <= 1+num_of_repetitions*2; i++)
						_free(stack[count-i]);
					count -= 1+num_of_repetitions*2;

					stack[count] = tmp;
					count++;
				}
			} else {
				if(get_operation_priority(expected_on_stack) < get_operation_priority(on_stack)) {
					// shift in next value
					stack[count] = stack[count-1]->next;
					count++;
				} else {
					// reduce
					token_t* tmp = token_create();
					tmp->type = TOKEN_TYPE_EXP;
					tmp->next = stack[count-1]->next;

					switch(on_stack) {
						case OPERATION_TYPE_VAR:
						case OPERATION_TYPE_STR:
							tmp->data.op = operation_create();
							tmp->data.op->type = on_stack;
							tmp->data.op->data.str = stack[count-1]->data.str;
							_free(stack[count-1]);
							count--;
							break;
						case OPERATION_TYPE_NUM:
							tmp->data.op = operation_create();
							tmp->data.op->type = on_stack;
							tmp->data.op->data.num = stack[count-1]->data.num;
							_free(stack[count-1]);
							count--;
							break;
						case OPERATION_TYPE_BOOL:
							tmp->data.op = operation_create();
							tmp->data.op->type = on_stack;
							tmp->data.op->data.boolean = stack[count-1]->data.boolean;
							_free(stack[count-1]);
							count--;
							break;
						case OPERATION_TYPE_NONE:
						case OPERATION_TYPE_READ:
						case OPERATION_TYPE_RAND:
							tmp->data.op = operation_create();
							tmp->data.op->type = on_stack;
							_free(stack[count-1]);
							count--;
							break;
						case OPERATION_TYPE_EXEC:
						case OPERATION_TYPE_INDEX:
							tmp->data.op = operation_create();
							tmp->data.op->type = on_stack;
							tmp->data.op->data.operations = (operation_t**)_alloc(sizeof(operation_t*)*2);
							tmp->data.op->data.operations[0] = stack[count-4]->data.op;
							tmp->data.op->data.operations[1] = stack[count-2]->data.op;
							_free(stack[count-4]);
							_free(stack[count-3]);
							_free(stack[count-2]);
							_free(stack[count-1]);
							count -= 4;
							break;
						case OPERATION_TYPE_IN_STRUCT:
						case OPERATION_TYPE_PAIR:
						case OPERATION_TYPE_ASSIGN:
						case OPERATION_TYPE_EQU:
						case OPERATION_TYPE_GTR:
						case OPERATION_TYPE_LES:
						case OPERATION_TYPE_LEQ:
						case OPERATION_TYPE_GEQ:
						case OPERATION_TYPE_FIND:
						case OPERATION_TYPE_SPLIT:
							tmp->data.op = operation_create();
							tmp->data.op->type = on_stack;
							tmp->data.op->data.operations = (operation_t**)_alloc(sizeof(operation_t*)*2);
							tmp->data.op->data.operations[0] = stack[count-3]->data.op;
							tmp->data.op->data.operations[1] = stack[count-1]->data.op;
							_free(stack[count-3]);
							_free(stack[count-2]);
							_free(stack[count-1]);
							count -= 3;
							break;
						case OPERATION_TYPE_NOOP_BRAC:
							tmp->data.op = stack[count-2]->data.op;
							_free(stack[count-3]);
							_free(stack[count-2]);
							_free(stack[count-1]);
							count -= 3;
							break;
						case OPERATION_TYPE_NOOP:
							stack[count-1]->type = TOKEN_TYPE_EXP;
							stack[count-1]->data.op = operation_create_NOOP();
							stack[count-1]->next = tmp;
							tmp->type = TOKEN_TYPE_CLOSE_BRAC;
							break;
						case OPERATION_TYPE_LIST:
						case OPERATION_TYPE_SCOPE:
						case OPERATION_TYPE_ABS:
							tmp->data.op = operation_create();
							tmp->data.op->type = on_stack;
							tmp->data.op->data.operations = (operation_t**)_alloc(sizeof(operation_t*));
							tmp->data.op->data.operations[0] = stack[count-2]->data.op;
							_free(stack[count-3]);
							_free(stack[count-2]);
							_free(stack[count-1]);
							count -= 3;
							break;
						case OPERATION_TYPE_NOOP_EMP_REC:
							tmp->data.op = operation_create();
							tmp->data.op->type = OPERATION_TYPE_LIST;
							tmp->data.op->data.operations = (operation_t**)_alloc(sizeof(operation_t*));
							tmp->data.op->data.operations[0] = operation_create_NOOP();
							_free(stack[count-2]);
							_free(stack[count-1]);
							count -= 2;
							break;
						case OPERATION_TYPE_NOOP_EMP_CUR: 
							tmp->data.op = operation_create();
							tmp->data.op->type = OPERATION_TYPE_SCOPE;
							tmp->data.op->data.operations = (operation_t**)_alloc(sizeof(operation_t*));
							tmp->data.op->data.operations[0] = operation_create_NOOP();
							_free(stack[count-2]);
							_free(stack[count-1]);
							count -= 2;
							break;
						case OPERATION_TYPE_IFELSE:
							tmp->data.op = operation_create();
							tmp->data.op->type = on_stack;
							tmp->data.op->data.operations = (operation_t**)_alloc(sizeof(operation_t*)*3);
							tmp->data.op->data.operations[0] = stack[count-5]->data.op;
							tmp->data.op->data.operations[1] = stack[count-3]->data.op;
							tmp->data.op->data.operations[2] = stack[count-1]->data.op;
							_free(stack[count-6]);
							_free(stack[count-5]);
							_free(stack[count-4]);
							_free(stack[count-3]);
							_free(stack[count-2]);
							_free(stack[count-1]);
							count -= 6;
							break;
						case OPERATION_TYPE_IF:
						case OPERATION_TYPE_WHILE:
						case OPERATION_TYPE_FUNCTION:
							tmp->data.op = operation_create();
							tmp->data.op->type = on_stack;
							tmp->data.op->data.operations = (operation_t**)_alloc(sizeof(operation_t*)*2);
							tmp->data.op->data.operations[0] = stack[count-3]->data.op;
							tmp->data.op->data.operations[1] = stack[count-1]->data.op;
							_free(stack[count-4]);
							_free(stack[count-3]);
							_free(stack[count-2]);
							_free(stack[count-1]);
							count -= 4;
							break;
						case OPERATION_TYPE_MACRO:
						case OPERATION_TYPE_STRUCT:
						case OPERATION_TYPE_DIC:
						case OPERATION_TYPE_SIN:
						case OPERATION_TYPE_COS:
						case OPERATION_TYPE_TAN:
						case OPERATION_TYPE_ASIN:
						case OPERATION_TYPE_ACOS:
						case OPERATION_TYPE_ATAN:
						case OPERATION_TYPE_SINH:
						case OPERATION_TYPE_COSH:
						case OPERATION_TYPE_TANH:
						case OPERATION_TYPE_ASINH:
						case OPERATION_TYPE_ACOSH:
						case OPERATION_TYPE_ATANH:
						case OPERATION_TYPE_TRUNC:
						case OPERATION_TYPE_FLOOR:
						case OPERATION_TYPE_CEIL:
						case OPERATION_TYPE_ROUND:
						case OPERATION_TYPE_LEN:
						case OPERATION_TYPE_CBRT:
						case OPERATION_TYPE_SQRT:
						case OPERATION_TYPE_TO_STR:
						case OPERATION_TYPE_TO_NUM:
						case OPERATION_TYPE_TO_BOOL:
						case OPERATION_TYPE_WRITE:
						case OPERATION_TYPE_NEG:
						case OPERATION_TYPE_NOT:
						case OPERATION_TYPE_LOCAL:
						case OPERATION_TYPE_GLOBAL:
						case OPERATION_TYPE_COPY:
							tmp->data.op = operation_create();
							tmp->data.op->type = on_stack;
							tmp->data.op->data.operations = (operation_t**)_alloc(sizeof(operation_t*));
							tmp->data.op->data.operations[0] = stack[count-1]->data.op;
							_free(stack[count-2]);
							_free(stack[count-1]);
							count -= 2;
							break;
						case OPERATION_TYPE_FOR:
							tmp->data.op = operation_create();
							tmp->data.op->type = on_stack;
							tmp->data.op->data.operations = (operation_t**)_alloc(sizeof(operation_t*)*4);
							tmp->data.op->data.operations[0] = stack[count-7]->data.op;
							tmp->data.op->data.operations[1] = stack[count-5]->data.op;
							tmp->data.op->data.operations[2] = stack[count-3]->data.op;
							tmp->data.op->data.operations[3] = stack[count-1]->data.op;
							_free(stack[count-8]);
							_free(stack[count-7]);
							_free(stack[count-6]);
							_free(stack[count-5]);
							_free(stack[count-4]);
							_free(stack[count-3]);
							_free(stack[count-2]);
							_free(stack[count-1]);
							count -= 8;
							break;
						case OPERATION_TYPE_NOOP_O_LIST_DEADEND:
						case OPERATION_TYPE_NOOP_PROC_DEADEND:
							tmp->data.op = stack[count-2]->data.op;
							_free(stack[count-2]);
							_free(stack[count-1]);
							count -= 2;
							break;
						default: /* this should never happen. */ break;
					}

					stack[count] = tmp;
					count++;
				}
			}
		}
	}

	if(stack[0]->type != TOKEN_TYPE_START || stack[1]->type != TOKEN_TYPE_EXP || stack[2]->type != TOKEN_TYPE_END)
		error("Parsing error.");
	program_t* ret = stack[1]->data.op;

	_free(stack[0]);
	_free(stack[1]);
	_free(stack[2]);
	_free(stack);
	return ret;
}

program_t* tokenize_and_parse_program(const char* src) {
	return parse_program(tokenize(src));
}

void program_free(program_t* program) {
	operation_free(program);
}

void program_exec(program_t* program, environment_t* env) {
	operation_exec(program, env);
}