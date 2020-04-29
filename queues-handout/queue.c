#include <stdlib.h>         // Standard C library: free(), NULL...  
#include <stdbool.h>        // Standard true, false, and bool type  
#include "lib/contracts.h"  // Our contracts library  
#include "lib/xalloc.h"     // Our allocation library  
#include "queue.h"          // The queue interface  

/**********************/
/*** Implementation ***/
/**********************/

typedef struct list_node list;
struct list_node {
  void* data;
  list* next;
};

typedef struct queue_header queue;
struct queue_header {
  list* front;
  list* back;
  int size;
};

bool is_inclusive_segment(list* start, list* end, int i)
//@requires i >= 0;
{   
    if(start == NULL) 
        return i == 0;
    if(start == end) 
        return i == 1 && start->next == NULL;
    return is_inclusive_segment(start->next, end, i - 1);
}


bool is_queue(queue_t Q) {
    if(Q == NULL)
      return false;
    if(Q->front == NULL)
      return Q->size == 0;
    if(Q->back == NULL)
      return Q->front == NULL && Q->size == 0;
    if(Q->front == Q->back)
      return Q->size == 1 && Q->back->next == NULL;
    if(Q->size > 0 && Q->back->next != NULL)
        return false;
    return is_inclusive_segment(Q->front, Q->back, Q->size);
}

queue_t queue_new()
//@ensures is_queue(\result);
{
    queue_t Q = xmalloc(sizeof(queue));
    Q->front = NULL;
    Q->back = NULL;
    Q->size = 0;
    ENSURES(is_queue(Q));
    return Q;
}

size_t queue_size(queue_t Q)                                /* O(1) */  
//@requires is_queue(Q);
//@ensures \result >= 0; 
{
    REQUIRES(is_queue(Q));
    ENSURES(Q->size >= 0); 
    return Q->size;
}

void enq(queue_t Q, void* x)
//@requires Q != NULL; 
//@ensures is_queue(Q);
{
    REQUIRES(is_queue(Q)); 
    list* new_elem = xmalloc(sizeof(list));
    new_elem->data = x;
    new_elem->next = NULL;
    if(Q->front == NULL)
        Q->front = new_elem;
    else
        Q->back->next = new_elem;
    Q->back = new_elem;
    Q->size = Q->size + 1;
    ENSURES(is_queue(Q));
}

void* deq(queue_t Q)
//@requires is_queue(Q) && queue_size(Q) > 0;
//@ensures is_queue(Q);
{
    REQUIRES(is_queue(Q) && queue_size(Q) > 0);
    if(Q->front == NULL)
        return NULL;
    if(queue_size(Q) == 1){
        void* x = Q->front->data;
        Q->front = NULL;
        Q->back = NULL;
        Q->size = 0;
        ENSURES(is_queue(Q));
        return x;
    }
    void* res = Q->front->data;
    Q->front = Q->front->next;
    Q->size = Q->size - 1;
    ENSURES(is_queue(Q));
    return res;
}

void *queue_peek(queue_t Q, size_t i)
//@requires is_queue(Q) && 0 <= i && i < queue_size(Q); 
{
    REQUIRES(is_queue(Q) && i < queue_size(Q));
    if(Q->front == NULL){
        assert(i == 0);
        return NULL;
    }
    list* R = Q->front;
    void* res = R->data;
    while((int)i >= 0){
        res = R->data;
        R = R->next;
        i -= 1;
    }
    ENSURES(is_queue(Q));
    return res;
}

void queue_reverse(queue_t Q)
//@requires is_queue(Q);
//@ensures is_queue(Q);
{
    REQUIRES(is_queue(Q));
    list* curr_node = Q->front;
    Q->back = curr_node;
    list* prev_node = NULL;
    list* temp_node = NULL;
    while(curr_node != NULL)
    {
        temp_node = curr_node->next;
        curr_node->next = prev_node;
        prev_node = curr_node;
        curr_node = temp_node;
    }
    Q->front = prev_node;
    ENSURES(is_queue(Q));
}

bool queue_all(queue_t Q, check_property_fn *P)
//@requires is_queue(Q) && P != NULL; 
{
    REQUIRES(is_queue(Q) && P != NULL);
    if(queue_size(Q) == 0) 
        return true;

    for(list* R = Q->front; R != Q->back->next; R = R->next)
    {
        if(!(*P)(R->data))
            return false;
    }
    ENSURES(is_queue(Q));
    return true;
}

void* queue_iterate(queue* Q, void* base, iterate_fn* F)
//@requires is_queue(Q) && F != NULL; 
{
    REQUIRES(is_queue(Q) && F != NULL);  
    if(Q->front == NULL && Q->size == 0){
        ENSURES(is_queue(Q));
        return base;
    }else{
        void* x = base;
        list* R = Q->front;
        for(int i = 0; i < Q->size; i++)
        {
            x = (*F)(x, R->data);
            R = R->next;
        }
        ENSURES(is_queue(Q));
        return x;
    }
    
}

void queue_free(queue_t Q, free_fn *F)
{   
    REQUIRES(is_queue(Q));
    if(F != NULL)
    {
        for(list* R = Q->front; R != NULL; R = R->next)
        {   
            if(!(R->data == NULL))
                (*F)(R->data);
        }
    }
    free(Q->front);
    free(Q->back);
    ENSURES(is_queue(Q));
}




/* Other functions go there */


// Client type

/*************************/
/*** Library interface ***/
/*************************/

// typedef ______* queue_t;

/* O(1) */
queue_t queue_new()
  /*@ensures \result != NULL; @*/ ;

/* O(1) */
size_t queue_size(queue_t Q)
  /*@requires Q != NULL; @*/
  /*@ensures \result >= 0; @*/ ;

/* O(1) -- adds an item to the back of the queue */
void enq(queue_t Q, void* x)
  /*@requires Q != NULL; @*/ ;

/* O(1) -- removes an item from the front of the queue */
void* deq(queue_t Q)
  /*@requires Q != NULL && queue_size(Q) > 0; @*/ ;

/* O(i) -- doesn't remove the item from the queue */
void *queue_peek(queue_t Q, size_t i)
  /*@requires Q != NULL && 0 <= i && i < queue_size(Q); @*/ ;

/* O(n) */
void queue_reverse(queue_t Q)
  /*@requires Q != NULL; @*/ ;

/* O(n) worst case, assuming P is O(1) */
bool queue_all(queue_t Q, check_property_fn* P)
  /*@requires Q != NULL && P != NULL; @*/ ;

/* O(n) worst case, assuming F is O(1) */
void* queue_iterate(queue_t Q, void* base, iterate_fn* F)
  /*@requires Q != NULL && F != NULL; @*/ ;