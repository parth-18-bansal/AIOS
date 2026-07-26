#include "types.h"

/*
summary: it is used to fill the value c in the page. so it take the starting address and then 
it takes the value that we want to fill and the number of the bytes that we want to fill
and then it fill that amount of the memory with the value c.
*/
void memset(void *dst, int c, uint n){
    char *cdst = (char *)dst;

    int i;
    for (i = 0; i<n; i++){
        cdst[i] = c;
    }

    return dst;
}
