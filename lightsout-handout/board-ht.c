#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "lib/contracts.h"
#include "lib/xalloc.h"
#include "lib/bitvector.h"
#include "lib/hdict.h"
#include "board-ht.h"


void* ht_key(void* e){   
    REQUIRES(e != NULL);
    struct board_data *new_e = (struct board_data *)e;
    return (void*)(&new_e->board); 
}

bool ht_equiv(void* k1, void* k2){

    return bitvector_equal(*(bitvector*)k1, *(bitvector*)k2);
}

size_t ht_hash(void* k)
{
    size_t res = 0;
    for (uint8_t i = 0; i < BITVECTOR_LIMIT; i++){
        bool b = bitvector_get(*(bitvector *)k, i);
        if(b) res = res + 1;
        res = res * 10;
    }
    return res; 
}

void ht_free(void* e){

    free((struct board_data*)e);
}

/* Initializes a new hash table with the given capacity */
hdict_t ht_new(size_t capacity){

    REQUIRES(0 < capacity);
    hdict_t H = hdict_new(capacity, &ht_key, &ht_equiv, &ht_hash, &ht_free);

    ENSURES(H != NULL);
    return H;
}

/* ht_lookup(H,B) returns 
 * NULL if no struct containing the board B exists in H
 * A struct containing the board B if one exists in H.
 */
struct board_data *ht_lookup(hdict_t H, bitvector B){

    REQUIRES(H != NULL);
    return (struct board_data *)hdict_lookup(H, (void*)(&B));
}

/* ht_insert(H,e) has no return value, because it should have as
 * a precondition that no struct currently in the hashtable contains
 * the same board as DAT->board.
 */
void ht_insert(hdict_t H, struct board_data *DAT){
    
    REQUIRES(H != NULL && DAT != NULL);
    REQUIRES((ht_lookup(H, DAT->board) == NULL));
    void * e = hdict_insert(H, (void*)DAT);
    if(e != NULL) free(e);
}
