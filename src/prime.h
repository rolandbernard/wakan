// Copyright (c) 2018 Roland Bernard

// SUB 100
#define TINY_PRIME_1 5
#define TINY_PRIME_2 29
#define TINY_PRIME_3 47
#define TINY_PRIME_4 53
#define TINY_PRIME_5 71

// SUB 1 000
#define SMALL_PRIME_1 311
#define SMALL_PRIME_2 487
#define SMALL_PRIME_3 557
#define SMALL_PRIME_4 787
#define SMALL_PRIME_5 997

// SUB 10 000
#define MEDIUM_PRIMES_1 3701
#define MEDIUM_PRIMES_2 6143
#define MEDIUM_PRIMES_3 7927
#define MEDIUM_PRIMES_4 8831
#define MEDIUM_PRIMES_5 9733

// SUB 1 000 000
#define BIG_PRIME_1 92387
#define BIG_PRIME_2 104999
#define BIG_PRIME_3 105601
#define BIG_PRIME_4 610877
#define BIG_PRIME_5 951161

// SUB 10 000 000
#define HUGE_PRIME_1 1301081
#define HUGE_PRIME_2 1323499
#define HUGE_PRIME_3 4256233
#define HUGE_PRIME_4 7368787
#define HUGE_PRIME_5 9763393

// SUB 1 000 000 000
/*
#define MASSIVE_PRIME_1
#define MASSIVE_PRIME_1
#define MASSIVE_PRIME_1
#define MASSIVE_PRIME_1
#define MASSIVE_PRIME_1
*/

// Max. prime in a 64bit signed integer
#define MAX_SLONG_PRIME 9223372036854775783L


unsigned int is_prime(const unsigned int x);
unsigned int next_prime(unsigned int x);