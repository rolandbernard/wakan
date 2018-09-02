
#include <math.h>

#include "prime.h"

unsigned int is_prime(const unsigned int x) {
    if (x < 2) { return -1; }
    if (x < 4) { return 1; }
    if ((x % 2) == 0) { return 0; }
    for (unsigned int i = 3; i <= (unsigned int)(sqrt((double) x)); i += 2) {
        if ((x % i) == 0) {
            return 0;
        }
    }
    return 1;
}

unsigned int next_prime(unsigned int x) {
    while (is_prime(x) != 1) {
        x++;
    }
    return x;
}