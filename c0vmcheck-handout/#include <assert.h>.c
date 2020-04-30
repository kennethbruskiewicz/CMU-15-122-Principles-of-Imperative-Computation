#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "lib/xalloc.h"
#include "lib/stack.h"
#include "lib/contracts.h"
#include "lib/c0v_stack.h"
#include "lib/c0vm.h"
#include "lib/c0vm_c0ffi.h"
#include "lib/c0vm_abort.h"

/* call stack frames */
typedef struct frame_info frame;
struct frame_info {
  c0v_stack_t S; /* Operand stack of C0 values */
  ubyte *P;      /* Function body */
  size_t pc;     /* Program counter */
  c0_value *V;   /* The local variables */
};


void push_int(c0v_stack_t S, int num) {
    c0v_push(S, int2val(num));
}

int pop_int(c0v_stack_t S){

    return val2int(c0v_pop(S));
}

void push_ptr(c0v_stack_t S, void* ptr){

    c0v_push(S, ptr2val(ptr));
}

void* pop_ptr(c0v_stack_t S) {

    return val2ptr(c0v_pop(S));
}

int execute(struct bc0_file *bc0) {
  REQUIRES(bc0 != NULL);

  /* Variables */
  c0v_stack_t S; /* Operand stack of C0 values */
  ubyte *P;      /* Array of bytes that make up the current function */
  size_t pc;     /* Current location within the current byte array P */
  c0_value *V;   /* Local variables (you won't need this till Task 2) */

  /* The call stack, a generic stack that should contain pointers to frames */
  /* You won't need this until you implement functions. */
  gstack_t callStack;
  callStack = stack_new();

  //Initialize S, P and pc
  S = c0v_stack_new();
  P = bc0->function_pool[0].code;
  pc = 0;
  V = xmalloc(sizeof(c0_value) * bc0->function_pool[0].num_vars);

  while (true) {

#ifdef DEBUG
    /* You can add extra debugging information here */
    fprintf(stderr, "Opcode %x -- Stack size: %zu -- PC: %zu\n",
            P[pc], c0v_stack_size(S), pc);
#endif

    switch (P[pc]) {

    /* Additional stack operation: */

    case POP: {
      pc++;
      c0v_pop(S);
      break;
    }

    case DUP: {
      pc++;
      c0_value v = c0v_pop(S);
      c0v_push(S,v);
      c0v_push(S,v);
      break;
    }

    case SWAP: {
      pc++;
      c0_value v1 = c0v_pop(S);
      c0_value v2 = c0v_pop(S);
      c0v_push(S,v1);
      c0v_push(S,v2);
      break;
    }


    /* Returning from a function.
     * This currently has a memory leak! You will need to make a slight
     * change for the initial tasks to avoid leaking memory.  You will
     * need to be revise it further when you write INVOKESTATIC. */

    case RETURN: {

      c0_value retval1 = c0v_pop(S);
      int retval2 = val2int(retval1);
      assert(c0v_stack_empty(S));

  #ifdef DEBUG
      fprintf(stderr, "Returning %d from execute()\n", retval2);
  #endif

      free(V); 
      c0v_stack_free(S); 
      // Free everything before returning from the execute function!
      if (stack_empty(callStack)) {
          stack_free(callStack, NULL);
          return retval2;
      }
      else {
          frame *R = (frame*) pop(callStack);
          V = R->V;
          S = R->S;
          P = R->P;
          pc = R->pc;
          free(R);
          c0v_push(S, retval1);
          break;
      }

    }

    case IADD:{
        pc++;
        int v1 = pop_int(S);
        int v2 = pop_int(S);
        push_int(S, v2+v1);
        break;
    }

    case ISUB: {
      pc++;
      int v1 = pop_int(S);
      int v2 = pop_int(S);
      push_int(S, v2-v1);
      break;
    }

    case IMUL:{
      pc++;
      int v1 = pop_int(S);
      int v2 = pop_int(S);
      push_int(S,(v2*v1));
      break;
    }

    case IDIV:{
      pc++;
      char *err = "Division Error\n";
      int v1 = pop_int(S);
      int v2 = pop_int(S);
      if(v1 == 0 || (v1 == -1 && v2 == INT_MIN))
        c0_arith_error(err);
      else push_int(S,(v2/v1));
      break;
    }

    case IREM:{
      pc++;
      char *err = "Division Error";
      int v1 = pop_int(S);
      int v2 = pop_int(S);
      if(v1 == 0 || (v1 == -1 && v2 == INT_MIN) )
        c0_arith_error(err);
      else push_int(S,(v2%v1));
      break;
    }

    case IAND:{
      pc++;
      int v1 = pop_int(S);
      int v2 = pop_int(S);
      push_int(S, v2&v1);
      break;
    }

    case IOR:{
      pc++;
      int v1 = pop_int(S);
      int v2 = pop_int(S);
      push_int(S, v2|v1);
      break;
    }

    case IXOR:{
      pc++;
      int v1 = pop_int(S);
      int v2 = pop_int(S);
      push_int(S, v2^v1);
      break;
    }

    case ISHL:{
      pc++;
      char *err = "Arithmetic Error";
      int v1 = pop_int(S);
      int v2 = pop_int(S);
      if(v1 < 0 || v1 >= 32) 
        c0_arith_error(err);
      else push_int(S, v2<<v1);
      break;
    }

    case ISHR:{
      pc++;
      char *err = "Arithmetic Error";
      int v1 = pop_int(S);
      int v2 = pop_int(S);
      if(v1 < 0 || v1 >= 32) 
        c0_arith_error(err);
      else push_int(S, v2>>v1);
      break;
    }


    /* Pushing constants */

    case BIPUSH:{
      pc++;
      c0_value v = int2val((int)(int8_t)P[pc]);
      c0v_push(S,v);
      pc++;
      break;
    }

    case ILDC: {
      pc++;
      uint16_t index1 = ((uint16_t)P[pc]) << 8;
      pc++;
      uint16_t index2 = (uint16_t)P[pc];
      int x = bc0->int_pool[index1 | index2];
      push_int(S, x);
      pc++;
      break;
    }

    case ALDC:{
      pc++;
      uint16_t index1 = ((uint16_t)P[pc]) << 8;
      pc++;
      uint16_t index2 = (uint16_t)P[pc];
      char *x = &bc0->string_pool[(index1 | index2)];
      push_ptr(S, (void*)x);
      pc++;
      break;
    }

    case ACONST_NULL: {
      pc++;
      void* ptr = 0;
      push_ptr(S, ptr);
      break;
    }

    /* Operations on local variables */

    case VLOAD:{
      pc++;
      c0v_push(S, V[P[pc]]);
      pc++;
      break;
    }

    case VSTORE:{
      pc++;
      V[P[pc]] = c0v_pop(S);
      pc++;
      break;
    }


    /* Assertions and errors */

    case ATHROW:{
      pc++;
      char *err = (char*)pop_ptr(S);
      c0_user_error(err);
      break;
    }

    case ASSERT:{
        pc++;
        char* err = (char*)pop_ptr(S);
        if (pop_int(S) == 0) 
          c0_assertion_failure(err);
        break;
    }


    /* Control flow operations */

    case NOP:{
      pc++;
      break;
    }

    case IF_CMPEQ:{
      pc++;
      c0_value v1 = c0v_pop(S); 
      c0_value v2 = c0v_pop(S); 
      if(val_equal(v1, v2)){
        uint16_t index1 = (uint16_t)P[pc] << 8;
        pc++;
        uint16_t index2 = (uint16_t)P[pc];
        int16_t index = (int16_t)(index1 | index2);
        if(index == 0) 
          pc++;
        else
          pc = pc + (index - 2);
      }
      else{
        pc += 2;
      }
      break;
    }

    case IF_CMPNE:{
      pc++;
      c0_value v1 = c0v_pop(S); 
      c0_value v2 = c0v_pop(S); 
      if(!val_equal(v1, v2)){
        uint16_t index1 = (uint16_t)P[pc] << 8;
        pc++;
        uint16_t index2 = (uint16_t)P[pc];
        int16_t index = (int16_t)(index1 | index2);
        if(index == 0) 
          pc++;
        else
          pc = pc + (index - 2);
      }
      else{
        pc += 2;
      }
      break;
    }

    case IF_ICMPLT:{
      pc++;
      int v1 = val2int(c0v_pop(S)); 
      int v2 = val2int(c0v_pop(S)); 
      if(v2 < v1){
        uint16_t index1 = (uint16_t)P[pc] << 8;
        pc++;
        uint16_t index2 = (uint16_t)P[pc];
        int16_t index = (int16_t)(index1 | index2);
        if(index == 0) 
          pc++;
        else
          pc = pc + (index - 2);
      }
      else{
        pc += 2;
      }
      break;
    }

    case IF_ICMPGE:{
      pc++;
      int v1 = val2int(c0v_pop(S)); 
      int v2 = val2int(c0v_pop(S)); 
      if(v2 >= v1){
        uint16_t index1 = (uint16_t)P[pc] << 8;
        pc++;
        uint16_t index2 = (uint16_t)P[pc];
        int16_t index = (int16_t)(index1 | index2);
        if(index == 0) 
          pc++;
        else
          pc = pc + (index - 2);
      }
      else{
        pc += 2;
      }
      break;
    }

    case IF_ICMPGT:{
      pc++;
      int v1 = val2int(c0v_pop(S)); 
      int v2 = val2int(c0v_pop(S)); 
      if(v2 > v1){
        uint16_t index1 = (uint16_t)P[pc] << 8;
        pc++;
        uint16_t index2 = (uint16_t)P[pc];
        int16_t index = (int16_t)(index1 | index2);
        if(index == 0) 
          pc++;
        else
          pc = pc + (index - 2);
      }
      else{
        pc += 2;
      }
      break;
    }

    case IF_ICMPLE:{
      pc++;
      int v1 = val2int(c0v_pop(S)); 
      int v2 = val2int(c0v_pop(S)); 
      if(v2 <= v1){
        uint16_t index1 = (uint16_t)P[pc] << 8;
        pc++;
        uint16_t index2 = (uint16_t)P[pc];
        int16_t index = (int16_t)(index1 | index2);
        if(index == 0) 
          pc++;
        else
          pc = pc + (index - 2);
      }
      else{
        pc += 2;
      }
      break;
    }

    case GOTO:{
      pc++;
      uint16_t index1 = (uint16_t)P[pc] << 8;
      pc++;
      uint16_t index2 = (uint16_t)P[pc];
      int16_t index = (int16_t)(index1 | index2);

      if (index == 0) 
        pc ++;
      else 
        pc = pc + (index - 2);
      break;
    }

    /* Function call operations: */

    case INVOKESTATIC:{

      frame* F = xmalloc(sizeof(frame));
      F->S = S;
      F->P = P;
      F->pc = pc + 3;
      F->V = V;
      push(callStack, (void*)F);

      pc++;
      uint16_t index1 = ((uint16_t)P[pc]) << 8;
      pc++;
      uint16_t index2 = (uint16_t)P[pc];

      struct function_info x = bc0->function_pool[(index1 | index2)];
      int arr_len = (int)x.num_vars;
      int arg_len = (int)x.num_args;

      V = xmalloc(sizeof(c0_value) * arr_len);
      for(int i = 0; i < arg_len; i++)
      {
        V[arg_len - 1 - i] = c0v_pop(S);
      }

      S = c0v_stack_new();
      P = x.code;
      pc = 0;

      break;
    }

    case INVOKENATIVE:{
      pc++;
      uint16_t index1 = ((uint16_t)P[pc]) << 8;
      pc++;
      uint16_t index2 = (uint16_t)P[pc];

      int arr_len = (int) bc0->native_pool[index1 | index2].num_args;
      uint16_t index = bc0->native_pool[index1 | index2].function_table_index;

      c0_value *A = xmalloc(sizeof(c0_value) * arr_len);
      for (int i = 0; i < arr_len; i++)
      {
          A[(arr_len - 1) - i] = c0v_pop(S);
      }

      native_fn *f = native_function_table[index];
      c0_value res = (*f)(A);
      c0v_push(S, res);
      pc++;

      break;
    }


    /* Memory allocation operations: */

    case NEW:{
      pc++;
      void *ptr = xmalloc((int)(int8_t)P[pc]);
      push_ptr(S, ptr);
      pc++;
      break;
    }

    case NEWARRAY:{
      pc++;
      c0_array *a = xmalloc(sizeof(c0_array));

      a->elt_size = (int)(int8_t)P[pc];
      a->count = pop_int(S);
      a->elems = xcalloc(a->count, a->elt_size);

      push_ptr(S, (void*)a);
      pc++;
      break;
    }

    case ARRAYLENGTH:{
      c0_array *a = pop_ptr(S);
      if (a == NULL) 
        c0_memory_error("Invalid array\n");
      push_int(S, a->count);
      pc++;
      break;
    }


    /* Memory access operations: */

    case AADDF:{
      pc++;
      ubyte f = P[pc];
      char *a = pop_ptr(S);

      if (a == NULL) 
        c0_memory_error("Invalid Pointer\n");

      push_ptr(S, (void*)(a+f));
      pc++;
      break;
    }

    case AADDS:{
      int i = pop_int(S);
      c0_array *a = (c0_array *)pop_ptr(S);
      if (a == NULL) 
        c0_memory_error("Invalid pointer\n");
      if (0 <= i && i < a->count) {
          push_ptr(S, (void *)(((char *)(a->elems))+(i * a->elt_size)));
          pc++;
          break;
      }
      else 
        c0_memory_error("Invalid Index!\n");
    }

    case IMLOAD:{
      int *a = pop_ptr(S);
      if (a == NULL) 
        c0_memory_error("Invalid Pointer\n");
      pc++;
      int x = *a;
      push_int(S, x);
      break;
    }

    case IMSTORE:{
      int *a = (int*)pop_ptr(S);
      if(a == NULL)
        c0_memory_error("Invalid Pointer\n");
      pc++;
      *a = pop_int(S);
      break;
    }

    case AMLOAD:{
      void **a = (void **)pop_ptr(S);
      if (a == NULL) 
        c0_memory_error("Invalid Pointer\n");
      pc++;
      void *b = *a;
      push_ptr(S, b);
      break;
    }

    case AMSTORE:{
      void *b = pop_ptr(S);
      void **a = (void **)pop_ptr(S);
      if (a == NULL) 
        c0_memory_error("Invalid Pointer\n");
      pc++;
      *a = b;
      break;
    }

    case CMLOAD:{
      char *a = pop_ptr(S);
      if (a == NULL) 
        c0_memory_error("Invalid Pointer\n");
      pc++;
      push_int(S, (int32_t)(*a));
      break;
    }

    case CMSTORE:{
        int x = pop_int(S);
        char* a = pop_ptr(S);
        if (a == NULL) c0_memory_error("pointer is NULL\n");
        pc++;
        *a = x & 0x7f;
        break;
    }

    default:
      fprintf(stderr, "invalid opcode: 0x%02x\n", P[pc]);
      abort();
    }
  }

  /* cannot get here from infinite loop */
  assert(false);
}
