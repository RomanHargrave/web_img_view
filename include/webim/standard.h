#pragma once
#define __GNU_SOURCE

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define wiv_new(x)  ((x*)(malloc(sizeof(x))))
#define wiv_free(x) {if (x) free(x);}

typedef char*           wiv_str;
typedef unsigned char   wiv_uint8;
typedef unsigned short  wiv_uint16;
typedef unsigned int    wiv_uint32;
