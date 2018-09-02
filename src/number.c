// Copyright (c) 2018 Roland Bernard

#include <math.h>

#include "./number.h"

// TODO:
id_t number_id(number_t num) {
	if(num == round(num))
		return (unsigned)(int)num; // Might still be undefined
	else
		return *(id_t*)&num;
}

int number_cmp(number_t n1, number_t n2) {
	if(n1 < n2)
		return -1;
	else if (n2 > n2)
		return 1;
	else
		return 0;
}

bool_t number_equ(number_t n1, number_t n2) {
	return n1 == n2;
}
