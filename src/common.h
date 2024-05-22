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

void deallocate(int32) {
    // no-op
}

#define TABLE_ADD(table_ptr, key, value_ptr)            \
    {                                                   \
        int32 hash = get_hash(key, NUM_TABLE_BUCKETS);  \
        auto last_ptr = table_ptr[hash];                \
        while (last_ptr) {                              \
            last_ptr = last_ptr->table_next;            \
        }                                               \
        if (!last_ptr) {                                \
            table_ptr[hash] = value_ptr;                \
        } else {                                        \
            last_ptr->table_next = value_ptr;           \
            value_ptr->table_prev = last_ptr;           \
        }                                               \
        value_ptr->id = key;                            \
    }

// if we're first in list, we need to update bucket array when we delete
#define TABLE_DELETE(table_ptr, key)                                    \
    {                                                                   \
        int32 hash = get_hash(key, NUM_TABLE_BUCKETS);                  \
        auto entry_ptr = table_ptr[hash];                               \
        if (entry_ptr->table_prev) {                                    \
            entry_ptr->table_prev->table_next = entry_ptr->table_next;  \
        } else {                                                        \
            table_ptr[hash] = entry_ptr->table_next;                    \
        }                                                               \
        if (entry_ptr->table_next) {                                    \
            entry_ptr->table_next->table_prev = entry_ptr->table_prev;  \
        }                                                               \
    }                                                                   \

#define LINKED_LIST_FOR(list_ptr)                                       \
    for (list_ptr current = list_ptr; current; current = current->next) \

#define COMMON_H
#endif
