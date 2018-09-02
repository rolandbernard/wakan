// Copyright (c) 2018 Roland Bernard

#include <math.h>

#include "./types.h"
#include "./tokenlist.h"
#include "./langallocator.h"
#include "./error.h"


#define TMP_STR_MAX 1<<16

bool_t cmp_str(const char* start, const char* end, const char* cmp) {
	int i;
	for(i = 0; i < end-start; i++) {
		if(cmp[i] == '\0')
			return false;
		if(start[i] != cmp[i])
			return false;
	}
	if(cmp[i] != '\0')
		return false;
	return true;
}

void add_simple_token(tokenlist_t* list, token_type_t type) {
	list->end->next = token_create();
	list->end->next->type = type;
	list->end->next->next = NULL;
	list->end = list->end->next;
}

void add_token(tokenlist_t* list, const char* start, const char* end) {
	if(cmp_str(start, end, "if")) {
		add_simple_token(list, TOKEN_TYPE_IF);
	} else if(cmp_str(start, end, "do")) {
		add_simple_token(list, TOKEN_TYPE_DO);
	} else if(cmp_str(start, end, "or")) {
		add_simple_token(list, TOKEN_TYPE_OR);
	} else if(cmp_str(start, end, "mod")) {
		add_simple_token(list, TOKEN_TYPE_MOD);
	} else if(cmp_str(start, end, "not")) {
		add_simple_token(list, TOKEN_TYPE_NOT);
	} else if(cmp_str(start, end, "and")) {
		add_simple_token(list, TOKEN_TYPE_AND);
	} else if(cmp_str(start, end, "xor")) {
		add_simple_token(list, TOKEN_TYPE_XOR);
	} else if(cmp_str(start, end, "for")) {
		add_simple_token(list, TOKEN_TYPE_FOR);
	} else if(cmp_str(start, end, "def")) {
		add_simple_token(list, TOKEN_TYPE_DEF);
	} else if(cmp_str(start, end, "len")) {
		add_simple_token(list, TOKEN_TYPE_LEN);
	} else if(cmp_str(start, end, "dic")) {
		add_simple_token(list, TOKEN_TYPE_DIC);
	} else if(cmp_str(start, end, "sin")) {
		add_simple_token(list, TOKEN_TYPE_SIN);
	} else if(cmp_str(start, end, "cos")) {
		add_simple_token(list, TOKEN_TYPE_COS);
	} else if(cmp_str(start, end, "tan")) {
		add_simple_token(list, TOKEN_TYPE_TAN);
	} else if(cmp_str(start, end, "asin")) {
		add_simple_token(list, TOKEN_TYPE_ASIN);
	} else if(cmp_str(start, end, "acos")) {
		add_simple_token(list, TOKEN_TYPE_ACOS);
	} else if(cmp_str(start, end, "atan")) {
		add_simple_token(list, TOKEN_TYPE_ATAN);
	} else if(cmp_str(start, end, "sinh")) {
		add_simple_token(list, TOKEN_TYPE_SINH);
	} else if(cmp_str(start, end, "cosh")) {
		add_simple_token(list, TOKEN_TYPE_COSH);
	} else if(cmp_str(start, end, "tanh")) {
		add_simple_token(list, TOKEN_TYPE_TANH);
	} else if(cmp_str(start, end, "find")) {
		add_simple_token(list, TOKEN_TYPE_FIND);
	} else if(cmp_str(start, end, "none")) {
		add_simple_token(list, TOKEN_TYPE_NONE);
	} else if(cmp_str(start, end, "then")) {
		add_simple_token(list, TOKEN_TYPE_THEN);
	} else if(cmp_str(start, end, "else")) {
		add_simple_token(list, TOKEN_TYPE_ELSE);
	} else if(cmp_str(start, end, "does")) {
		add_simple_token(list, TOKEN_TYPE_DOES);
	} else if(cmp_str(start, end, "true")) {
		add_simple_token(list, TOKEN_TYPE_BOOL);
		list->end->data.boolean = true;
	} else if(cmp_str(start, end, "sqrt")) {
		add_simple_token(list, TOKEN_TYPE_SQRT);
	} else if(cmp_str(start, end, "cbrt")) {
		add_simple_token(list, TOKEN_TYPE_CBRT);
	} else if(cmp_str(start, end, "ceil")) {
		add_simple_token(list, TOKEN_TYPE_CEIL);
	} else if(cmp_str(start, end, "rand")) {
		add_simple_token(list, TOKEN_TYPE_RAND);
	} else if(cmp_str(start, end, "read")) {
		add_simple_token(list, TOKEN_TYPE_READ);
	} else if(cmp_str(start, end, "copy")) {
		add_simple_token(list, TOKEN_TYPE_COPY);
	} else if(cmp_str(start, end, "split")) {
		add_simple_token(list, TOKEN_TYPE_SPLIT);
	} else if(cmp_str(start, end, "local")) {
		add_simple_token(list, TOKEN_TYPE_LOCAL);
	} else if(cmp_str(start, end, "round")) {
		add_simple_token(list, TOKEN_TYPE_ROUND);
	} else if(cmp_str(start, end, "trunc")) {
		add_simple_token(list, TOKEN_TYPE_TRUNC);
	} else if(cmp_str(start, end, "floor")) {
		add_simple_token(list, TOKEN_TYPE_FLOOR);
	} else if(cmp_str(start, end, "write")) {
		add_simple_token(list, TOKEN_TYPE_WRITE);
	} else if(cmp_str(start, end, "false")) {
		add_simple_token(list, TOKEN_TYPE_BOOL);
		list->end->data.boolean = false;
	} else if(cmp_str(start, end, "asinh")) {
		add_simple_token(list, TOKEN_TYPE_ASINH);
	} else if(cmp_str(start, end, "acosh")) {
		add_simple_token(list, TOKEN_TYPE_ACOSH);
	} else if(cmp_str(start, end, "atanh")) {
		add_simple_token(list, TOKEN_TYPE_ATANH);
	} else if(cmp_str(start, end, "while")) {
		add_simple_token(list, TOKEN_TYPE_WHILE);
	} else if(cmp_str(start, end, "global")) {
		add_simple_token(list, TOKEN_TYPE_GLOBAL);
	} else if(cmp_str(start, end, "struct")) {
		add_simple_token(list, TOKEN_TYPE_STRUCT);
	} else if(cmp_str(start, end, "to_num")) {
		add_simple_token(list, TOKEN_TYPE_TO_NUM);
	} else if(cmp_str(start, end, "to_str")) {
		add_simple_token(list, TOKEN_TYPE_TO_STR);
	} else if(cmp_str(start, end, "to_bool")) {
		add_simple_token(list, TOKEN_TYPE_TO_BOOL);
	} else if(cmp_str(start, end, "function")) {
		add_simple_token(list, TOKEN_TYPE_FUNCTION);
	} else {
		add_simple_token(list, TOKEN_TYPE_VAR);
		list->end->data.str = string_create_full(start, end-start);
	}
}

tokenlist_t* tokenize(const char* src) {
	tokenlist_t* ret = (tokenlist_t*)_alloc(sizeof(tokenlist_t));
	ret->start = token_create();
	ret->start->type = TOKEN_TYPE_START;
	ret->end = ret->start;
	const char* cur_pos = src;
	const char* start_pos = src;

	while(*cur_pos != '\0') {
		if(*cur_pos == ' ' || *cur_pos == '\t' || *cur_pos == '\n' || *cur_pos == '\0') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			start_pos = cur_pos+1;
		} else if(*cur_pos == '.') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_DOT);
			start_pos = cur_pos+1;
		} else if(*cur_pos == '+') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_PLUS);
			start_pos = cur_pos+1;
		} else if(*cur_pos == '-') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_MINUS);
			start_pos = cur_pos+1;
		} else if(*cur_pos == '*') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_MUL);
			start_pos = cur_pos+1;
		} else if(*cur_pos == '\\') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_BACKSLASH);
			start_pos = cur_pos+1;
		} else if(*cur_pos == '/') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_DIV);
			start_pos = cur_pos+1;
		} else if(*cur_pos == '^') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_POW);
			start_pos = cur_pos+1;
		} else if(*cur_pos == ':') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);

			if(*(cur_pos+1) == '=') {
				add_simple_token(ret, TOKEN_TYPE_ASSIGN);
				start_pos = cur_pos+2;
				cur_pos++;
			} else {
				add_simple_token(ret, TOKEN_TYPE_PAIR);
				start_pos = cur_pos+1;
			}
		} else if(*cur_pos == '=') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_EQU);
			start_pos = cur_pos+1;
		} else if(*cur_pos == '>') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);

			if(*(cur_pos+1) == '=') {
				add_simple_token(ret, TOKEN_TYPE_GEQ);
				start_pos = cur_pos+2;
				cur_pos++;
			} else {
				add_simple_token(ret, TOKEN_TYPE_GTR);
				start_pos = cur_pos+1;
			}
		} else if(*cur_pos == '<') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);

			if(*(cur_pos+1) == '=') {
				add_simple_token(ret, TOKEN_TYPE_LEQ);
				start_pos = cur_pos+2;
				cur_pos++;
			} else {
				add_simple_token(ret, TOKEN_TYPE_LES);
				start_pos = cur_pos+1;
			}
		} else if(*cur_pos == ';') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_SEMICOL);
			start_pos = cur_pos+1;
		} else if(*cur_pos == ',') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_COMMA);
			start_pos = cur_pos+1;
		} else if(*cur_pos == '(') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_OPEN_BRAC);
			start_pos = cur_pos+1;
		} else if(*cur_pos == ')') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_CLOSE_BRAC);
			start_pos = cur_pos+1;
		} else if(*cur_pos == '[') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_OPEN_REC);
			start_pos = cur_pos+1;
		} else if(*cur_pos == ']') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_CLOSE_REC);
			start_pos = cur_pos+1;
		} else if(*cur_pos == '{') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_OPEN_CUR);
			start_pos = cur_pos+1;
		} else if(*cur_pos == '}') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_CLOSE_CUR);
			start_pos = cur_pos+1;
		} else if(*cur_pos == '|') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);
			add_simple_token(ret, TOKEN_TYPE_ABS);
			start_pos = cur_pos+1;
		} else if(*cur_pos == '\"' || *cur_pos == '\'') {
			if(start_pos != cur_pos)
				add_token(ret, start_pos, cur_pos);

			char temp_str[TMP_STR_MAX];

			size_t length = 0;
			int index = 1;
			while(*(cur_pos+index) != *cur_pos) {
				char c = *(cur_pos+index);
				index++;

				if(c == '\0')
					error("Tokenization error: Unexpected end of input.");

				if(c == '\\') {
					c = *(cur_pos+index);
					index++;
					switch(c) {
						case '\0': error("Tokenization error: Unexpected end of input."); break;
						case '0': c = '\0'; break;
						case 'a': c = '\a'; break;
						case 'b': c = '\b'; break;
						case 't': c = '\t'; break;
						case 'n': c = '\n'; break;
						case 'v': c = '\v'; break;
						case 'f': c = '\f'; break;
						case 'r': c = '\r'; break;
						default: break;
					}
				}

				temp_str[length] = c;
				length++;	
			}

			add_simple_token(ret, TOKEN_TYPE_STR);
			ret->end->data.str = string_create_full(temp_str, length);

			cur_pos += index;
			start_pos = cur_pos+1;
		} else if(*cur_pos >= '0' && *cur_pos <= '9') {
			number_t num = 0;
			long div = 1;
			int exp = 0;
			bool_t neg_exp = false, dot = false, in_exp = false;

			while((*cur_pos >= '0' && *cur_pos <= '9') || *cur_pos == '.' || *cur_pos == 'e') {
				if(*cur_pos == '.')
					dot = true;
				else if(*cur_pos == 'e') {
					if(*(cur_pos+1) == '-' || *(cur_pos+1) == '+') {
						cur_pos++;
						neg_exp = *cur_pos == '-';
					}
					in_exp = true;
				} else {
					if(in_exp) {
						exp *= 10;
						exp += (number_t)(*cur_pos - '0');
					} else {
						if(dot)
							div *= 10;
						num *= 10;
						num += (number_t)(*cur_pos - '0');
					}
				}
				cur_pos++;
			}
			start_pos = cur_pos;
			cur_pos--;

			num /= div;
			num *= pow(10, neg_exp ? -exp : exp);

			add_simple_token(ret, TOKEN_TYPE_NUM);
			ret->end->data.num = num;
		} else if(*cur_pos == '#') {
			cur_pos++;
			while(*cur_pos != '\n' && *cur_pos != '#' && *cur_pos != '\0') cur_pos++;
			start_pos = cur_pos+1;
		}
		cur_pos++;
	}
	if(start_pos != cur_pos)
		add_token(ret, start_pos, cur_pos);

	add_simple_token(ret, TOKEN_TYPE_END);

	return ret;
}

void tokenlist_free(tokenlist_t* list) {
	while(list->start->next != NULL) {
		token_t* tmp = list->start->next;
		_free(list->start);
		list->start = tmp;
	}
	_free(list);
}