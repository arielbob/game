#ifndef COMMON_H

#define global_variable static
#define local_persist static
#define internal static

#if GAME_SLOW
#define assert(expression) if (!(expression)) { *(int *)0 = 0; }
#else
#define assert(expression)
#endif

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
