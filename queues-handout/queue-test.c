#include <stdlib.h>         // Standard C library: free(), NULL...  
#include <stdbool.h>        // Standard true, false, and bool type  
#include "lib/contracts.h"  // Our contracts library  
#include "lib/xalloc.h"     // Our allocation library  
#include "queue.h"          // The queue interface  
bool is_queue(queue_t Q);

int main() {
  // Create a few queues
  queue_t A = queue_new();
  ASSERT(is_queue(A));
  int* a1 = xmalloc(sizeof(int));
  *a1 = 8;
  int* a2 = xmalloc(sizeof(int));
  *a2 = 5;
  int* a3 = xmalloc(sizeof(int));
  *a3 = 15;
  enq(A, (void*)a1);
  enq(A, (void*)a2);
  enq(A, (void*)a3);
  ASSERT(queue_size(A) == 3);
  ASSERT(*(int*)deq(A) == 8);
  ASSERT(queue_size(A) == 2);
  ASSERT(*(int*)queue_peek(A, 0) == 5);
  ASSERT(*(int*)queue_peek(A, 1) == 15);
  ASSERT(is_queue(A));
  //print_queue(A, &print_elem);
  queue_reverse(A);
  free(a1);
  free(a2);
  free(a3);
  free(A);
  printf("All tests passed!");
  return 0;
}