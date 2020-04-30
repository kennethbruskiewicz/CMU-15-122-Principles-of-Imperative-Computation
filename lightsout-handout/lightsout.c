#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lib/xalloc.h"
#include "lib/contracts.h"
#include "lib/bitvector.h"
#include "board-ht.h"
#include "lib/queue.h"
#include "lib/hdict.h"
#include "lib/boardutil.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: lightsout <board name>\n");
        return 1;
    }


    bitvector b = bitvector_new();
    bitvector s = bitvector_new();
    uint8_t row = 0; 
    uint8_t col = 0;

    if (!file_read(argv[1], &b, &col, &row)) {
        fprintf(stderr, "Failed to load file\n");
        return 1;
    }

    struct board_data *init_board = xmalloc(sizeof(struct board_data));
    init_board->board = b;
    init_board->solution = s;

    queue_t Q  = queue_new();
    hdict_t H = ht_new((size_t)(row * col * 100));
    enq(Q, (void*)init_board);
    ht_insert(H, init_board);

    while (!queue_empty(Q)) {

        struct board_data *B = (struct board_data*) deq(Q);

        b = B->board;
        s = B->solution;

        for (int r = 0; r < row; r++) {
            for (int c = 0; c < col; c++) {
                
                //press_button
                uint8_t i = get_index(r, c, col, row);
                bitvector new_b = bitvector_flip(b, i);

                if(is_valid_pos(r, c+1, col, row)){
                    new_b = bitvector_flip(new_b, get_index(r, c+1, col, row));
                }
                if(is_valid_pos(r, c-1, col, row)){
                    new_b = bitvector_flip(new_b, get_index(r, c - 1, col, row));
                }
                if(is_valid_pos(r+1, c, col, row)){
                    new_b = bitvector_flip(new_b, get_index(r+1, c, col, row));
                }
                if(is_valid_pos(r-1, c, col, row)){
                    new_b = bitvector_flip(new_b, get_index(r-1, c, col, row));
                }

                b = new_b;

                //new_sol
                s = bitvector_flip(s, i);

                //if number of lights == 0
                if (bitvector_equal(b, bitvector_new())) {

                    for (uint8_t j = 0; j < BITVECTOR_LIMIT; j++) {
                        if (bitvector_get(s, j)){
                            fprintf(stdout, "%d:%d\n", j / col, j % col);
                        }
                    }
                    
                    fprintf(stderr, "Solvable!\n");
                    queue_free(Q, NULL);
                    hdict_free(H);
                    return 0;
                }

                //add board to hdict if not in hdict
                if (ht_lookup(H, b) == NULL) {

                    struct board_data *newboard = xmalloc(sizeof(struct board_data));
                    newboard->board = b;
                    newboard->solution = s;
                    enq(Q, (void*) newboard);
                    ht_insert(H, newboard);
                    
                }
            }
        }
    }

    fprintf(stderr, "Not solvable\n");
    queue_free(Q, NULL);
    hdict_free(H);
    return 1;
}