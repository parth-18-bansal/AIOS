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

/*
string comparison, means we are checking are first n character of the strings equal or not
*/
int strncmp(const char *p, const char *q, uint n){
    while(n > 0 && *p && *p == *q){
        n--, p++, q++;
    }

    if(n == 0){
        return 0;
    }

    return (uchar)*p - (uchar)*q;
}

/*
s = destination
t = source
n = number of character to copy

it copy the n character from t to s.

*/
char *strncpy(char *s, const char *t, int n){
    // pointer s will change so storing it's starting value in os

    char *os;
    os = s;

    /*
    copying n character from t to s

    *s++ = *t++ means
    *s = *t
    and then s++ and t++
    */
    while(n-- > 0 && (*s++ = *t++) != 0)
        ;

    /*
    if character is t get finish and still n > 0 then append \0 in the s
    */
    while(n-- > 0){
        *s++ = 0;
    }

    // return the starting of string s.
    return os;
}