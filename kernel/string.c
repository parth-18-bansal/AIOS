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


/*
summary:
it is to copy the n bytes from the src memory point and paste those bytes at the dst memory point
*/
void *memmove(void *dst, const void *src, uint n){
    const char *s;
    char *d;

    if(n==0){
        return dst;
    }

    s = src;
    d = dst;

    if(s < d && s+n > d){
        s+=n;
        d+=n;

        while(n-- > 0){
            *--d = *--s;
        }
    }
    else{
        while(n-- > 0){
            *d++ = *s++;
        }
    }

    return dst;
}

void *memcpy(void *dst, const void *src, uint n){
    return memmove(dst, src, n);
}
