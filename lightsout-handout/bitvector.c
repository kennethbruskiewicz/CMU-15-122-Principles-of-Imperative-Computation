#include "lib/bitvector.h"
#include "lib/xalloc.h"
#include "lib/contracts.h"
#include <stdlib.h>


/* Get a new bitvector with everything set to 'false'. */
bitvector bitvector_new(){
    return (bitvector)0;

}

/* Get the ith bit of the bitvector n. */
bool bitvector_get(bitvector B, uint8_t i){

    REQUIRES(i < BITVECTOR_LIMIT && 0 <= BITVECTOR_LIMIT);
    return ((B >> i) & (bitvector)1) == (bitvector)1;

}

/* Toggle the ith bit of the bitvector n, returning a new bitvector. */
/* The old bitvector remains unchanged. */
bitvector bitvector_flip(bitvector B, uint8_t i){

    REQUIRES(i < BITVECTOR_LIMIT && 0 <= BITVECTOR_LIMIT);
    return B ^ ((bitvector)1 << i);

}

/* Compare two bitvectors for equality. */
bool bitvector_equal(bitvector B1, bitvector B2){

    return (B1 ^ B2)  == (bitvector)0;

}
