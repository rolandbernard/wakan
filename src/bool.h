#ifndef __BOOL_H__
#define __BOOL_H__

#include "./types.h"

typedef unsigned char bool_t;

#define true 1
#define false 0

id_t bool_id(bool_t b);
bool_t bool_equ(bool_t b1, bool_t b2);

#endif