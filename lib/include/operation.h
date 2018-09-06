// Copyright (c) 2018 Roland Bernard

#ifndef __OPERATION_H__
#define __OPERATION_H__

#include "./types.h"
#include "./number.h"
#include "./bool.h"
#include "./string.h"
#include "./environment.h"

#define RET_ERROR (void*)0x01

typedef enum operation_type_e {
	OPERATION_TYPE_NOOP,	// ()
	OPERATION_TYPE_NUM,		// 12
	OPERATION_TYPE_STR,		// "hello"
	OPERATION_TYPE_VAR,		// a
	OPERATION_TYPE_BOOL,	// true
	OPERATION_TYPE_NONE,	// none

	OPERATION_TYPE_PAIR,	// 5:true		// EXP : EXP
	OPERATION_TYPE_FUNCTION,// function (end := "\n";str) does (print to_str str + end)	// function EXP does EXP
	OPERATION_TYPE_MACRO,	// def 5 + x		// def EXP
	OPERATION_TYPE_ASSIGN,	// a := 5		// EXP := EXP
	OPERATION_TYPE_PROC,	// a := 5; b := 6; c := a + b	// EXP ; EXP
	OPERATION_TYPE_STRUCT,	// struct (a := 5;b;c)		// struct EXP
	OPERATION_TYPE_O_LIST,	// 5,5		// EXP , EXP
	OPERATION_TYPE_LIST,	// [5,5]		// [ EXP ]
	OPERATION_TYPE_INDEX,	// a[5]		// EXP [ EXP ]
	OPERATION_TYPE_EXEC,	// a(5)		// EXP ( EXP )
	OPERATION_TYPE_TO_NUM,	// to_num "12"		// to_num EXP
	OPERATION_TYPE_TO_BOOL,	// to_bool 0		// to_bool EXP
	OPERATION_TYPE_TO_STR,	// to_str 12		// to_str EXP
	OPERATION_TYPE_READ,	// read		// read
	OPERATION_TYPE_WRITE,	// write str 12		// write EXP
	OPERATION_TYPE_ADD,		// 5 + 5		// EXP + EXP
	OPERATION_TYPE_SUB,		// 5 - 4		// EXP - EXP
	OPERATION_TYPE_MUL,		// 5 * 5		// EXP * EXP
	OPERATION_TYPE_DIV,		// 5 / 4		// EXP / EXP
	OPERATION_TYPE_MOD,		// 5 mod 2		// EXP mod EXP
	OPERATION_TYPE_NEG,		// -5		// !EXP - EXP
	OPERATION_TYPE_POW,		// 5 ^ 2		// EXP ^ EXP
	OPERATION_TYPE_AND,		// true and a		// EXP and EXP
	OPERATION_TYPE_OR,		// true or false		// EXP or EXP
	OPERATION_TYPE_XOR,		// true xor false		// EXP xor EXP
	OPERATION_TYPE_NOT,		// not true		// not EXP
	OPERATION_TYPE_SQRT,	// sqrt 36		// sqrt EXP
	OPERATION_TYPE_CBRT,	// cbrt 8		// cbrt EXP	
	OPERATION_TYPE_SIN,		// sin 12		// sin EXP
	OPERATION_TYPE_COS,		// cos 12
	OPERATION_TYPE_TAN,		// tan 12
	OPERATION_TYPE_ASIN,	// arcsin 12
	OPERATION_TYPE_ACOS,	// arccos 12
	OPERATION_TYPE_ATAN,	// arctan 12
	OPERATION_TYPE_SINH, 	// sinh 12
	OPERATION_TYPE_COSH,	// cosh 12
	OPERATION_TYPE_TANH,	// tanh 12
	OPERATION_TYPE_ASINH,	// arcsinh 12
	OPERATION_TYPE_ACOSH,	// arccosh 12
	OPERATION_TYPE_ATANH,	// arctanh 12
	OPERATION_TYPE_TRUNC,	// trunc -1.3
	OPERATION_TYPE_FLOOR,	// floor 12.5
	OPERATION_TYPE_CEIL,	// ceil 12.6
	OPERATION_TYPE_ROUND,	// round 12.2
	OPERATION_TYPE_RAND,	// rand
	OPERATION_TYPE_LEN,		// len 12
	OPERATION_TYPE_EQU,		// 5 = 4
	OPERATION_TYPE_GEQ,		// 5 >= 4
	OPERATION_TYPE_LEQ,		// 5 <= 4
	OPERATION_TYPE_GTR,		// 5 > 4
	OPERATION_TYPE_LES,		// 5 < 4
	OPERATION_TYPE_DIC,		// dic("hallo":2,"no":2)		// dic EXP
	OPERATION_TYPE_FIND,	// "hallo2hallo" find "2"		// EXP find EXP
	OPERATION_TYPE_SPLIT,	// "hallo2hallo" split "2"		// EXP split EXP
	OPERATION_TYPE_ABS,		// | -12 |		// | EXP |
	OPERATION_TYPE_SCOPE,	// {a := 3}		// { EXP }
	OPERATION_TYPE_IF,		// if (true or false) then (a := 3)		// if EXP then EXP
	OPERATION_TYPE_IFELSE,	// if (true and false) then (true and true) else (a := 3)		// if EXP then EXP else EXP
	OPERATION_TYPE_WHILE,	// while true do sum := sum + 1		// while EXP do EXP
	OPERATION_TYPE_IN_STRUCT,// (struct (x := none, y := 5)).x := 17		// EXP . EXP
	OPERATION_TYPE_NOOP_BRAC,	// (Exp)
	OPERATION_TYPE_NOOP_EMP_REC,	// []
	OPERATION_TYPE_NOOP_EMP_CUR,	// {}
	OPERATION_TYPE_LOCAL,		// local E
	OPERATION_TYPE_GLOBAL,		// global E
	OPERATION_TYPE_COPY,		// copy e
	OPERATION_TYPE_NOOP_O_LIST_DEADEND,		// E,
	OPERATION_TYPE_NOOP_PROC_DEADEND,		// E;
	OPERATION_TYPE_FOR,		// for E\E\E do E
	OPERATION_TYPE_NOOP_PLUS,		// +E
	OPERATION_TYPE_TO_ASCII,		// to_ascii E
} operation_type_t;

typedef struct operation_s {
	operation_type_t type;
	union operation_data_u {
		number_t num;
		bool_t boolean;
		string_t* str;
		struct operation_s**  operations;
	} data;
} operation_t;

operation_t* operation_create();
operation_t* operation_create_NOOP();
void* operation_exec(operation_t* op, environment_t* env);
object_t** operation_result(operation_t* op, environment_t* env);
object_t*** operation_var(operation_t* op, environment_t* env);
void operation_free(operation_t* op);
id_t operation_id(operation_t* op);
bool_t operation_equ(operation_t* o1, operation_t* o2);

#endif