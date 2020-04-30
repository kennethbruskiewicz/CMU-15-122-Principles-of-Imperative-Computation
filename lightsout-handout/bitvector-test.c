#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "lib/xalloc.h"
#include "lib/contracts.h"
#include "lib/bitvector.h"
#include "board-ht.h"

int main() {
  // Create a few queues
  bitvector x1 = bitvector_new();
  bitvector x2 = bitvector_new();
  printf("%d:%d\n", x1, x2);
  ASSERT(bitvector_equal(x1, x2));

  printf("%d\n", x1);
  x1 = bitvector_flip(x1, 0);
  printf("%d\n", x1);
  ASSERT(bitvector_get(x1, 0));
  ASSERT(!bitvector_get(x1, 1));

  // Using them, test the functions you wrote in file bitvector.c
  printf("All tests passed!\n");
  return 0;
}
