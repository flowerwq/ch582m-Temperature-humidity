#ifndef _MY_STDTYPE_H_
#define _MY_STDTYPE_H_
#include <stdint.h>
#include "CH58x_common.h"

//#if !defined(TRUE)
//#define TRUE (1)
//#endif 
//#define FALSE (0)

typedef uint32_t MY_BOOL;
typedef float MY_FLT32;
typedef uint8_t MY_U8;
typedef uint16_t MY_U16;
typedef uint32_t MY_U32;
typedef int16_t MY_INT16;

// armcc 
#define _MY_INLINE_ __inline
#define _MY_EXTERN_ extern
// GNU Compiler 

//#include <stdlib.h>
//#include <assert.h>
#include <stddef.h>
#define MY_ASSERT assert

typedef enum {
    FALSE = 0,
    TRUE = !FALSE
} bool;




#endif //MY_STDTYPE_H
