#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lib/contracts.h"
int main()
{
    int the_int = (-247);
    //char the_int2 = (char)the_int1;
    //short the_int3 = (short)the_int2;
    //unsigned short the_int4 = (unsigned short)the_int3;
    //printf("%d/n", the_int);
    printf("%x\n", (unsigned int)the_int);
    //printf("%d/n", the_int2);
    //printf("%d/n", the_int3);
    //printf("%d/n", the_int4);
}