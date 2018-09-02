// Copyright (c) 2018 Roland Bernard

#include "./bool.h"

id_t bool_id(bool_t b)
{
	if(b)
		return 1;
	else
		return 0;
}

bool_t bool_equ(bool_t b1, bool_t b2) {
	if((b1 && b2) || (!b1 && !b2))
		return 1;
	else 
		return 0;
}
