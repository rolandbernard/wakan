
#include "./token.h"
#include "./langallocator.h"

token_t* token_create() {
	return (token_t*)_alloc(sizeof(token_t));
}
