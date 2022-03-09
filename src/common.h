#ifndef COMMON_H

#include <assert.h>

#define global_variable static
#define local_persist static
#define internal static

#define PI 3.14159265358979323846f

#if 0
#if GAME_SLOW
#define assert(expression) { if (!(expression)) *(int *)0 = 0; }
#else
#define assert(expression)
#endif
#endif

#define array_length(array) sizeof(array) / sizeof((array)[0])

#define KILOBYTES(n) 1024 * (n)
#define MEGABYTES(n) 1024 * KILOBYTES(n)
#define GIGABYTES(n) 1024 * MEGABYTES(n)

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

typedef uint32 bool32;

#define COMMON_H
#endif
