
#include "./stdlib.h"
#include "./stdio.h"
#include "./error.h"

void error(const char* str) {
	fprintf(stderr, "Error: %s (Aborting process)\n", str);
	abort();
}