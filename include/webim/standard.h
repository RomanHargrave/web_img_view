#pragma once
#define __GNU_SOURCE

#define wiv_new(x)  ((x*)(malloc(sizeof(x))))
#define wiv_free(x) {if (x) free(x);}

typedef char*           wiv_str;
typedef unsigned char   wiv_uint8;
typedef unsigned short  wiv_uint16;
typedef unsigned int    wiv_uint32;
