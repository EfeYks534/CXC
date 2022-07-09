#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>


#define TRUE  ((bool)1)
#define FALSE ((bool)0)
#define ON    ((bool)1)
#define OFF   ((bool)0)


typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef signed long long int64;
typedef signed int int32;
typedef signed short int16;
typedef signed char int8;

typedef int8 bool;

_Static_assert(sizeof(uint64) == 8, "integer of wrong size");
_Static_assert(sizeof(uint32) == 4, "integer of wrong size");
_Static_assert(sizeof(uint16) == 2, "integer of wrong size");
_Static_assert(sizeof(uint8)  == 1, "integer of wrong size");
_Static_assert(sizeof(int64)  == 8, "integer of wrong size");
_Static_assert(sizeof(int32)  == 4, "integer of wrong size");
_Static_assert(sizeof(int16)  == 2, "integer of wrong size");
_Static_assert(sizeof(int8)   == 1, "integer of wrong size");


void *Alloc(uint64 size);

void *AllocZ(uint64 size);
