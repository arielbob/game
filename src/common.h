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

// table_ptr is just the array of entries.. i.e. the table.
// result is a variable of type of the contents of the table
// for example, for Mesh *mesh_table, you would do
// Mesh *current;
// TABLE_FIND(mesh_table, id, current);
#define TABLE_FIND(table_ptr, key_name, key, result)     \
    {                                                    \
        result = NULL;                                   \
        uint32 hash = get_hash(key, NUM_TABLE_BUCKETS);   \
                                                         \
        auto current = table_ptr[hash];                  \
        while (current) {                                \
            if (current->key_name == key) {              \
                result = current;                        \
                break;                                   \
            }                                            \
                                                         \
            current = current->table_next;               \
        }                                                \
    }                                                    \

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

// if we're first in list, we need to update bucket array when we delete.
// (we do this in the macro; the above comment is meant to be inside the
// macro, but you can't put comments inside..)
// note that this doesn't deallocate the entry; it just removes it from
// the hash table.
// k is key; had to rename it from key because table entries have a member
// called key.. and it was replacing it with the macro argument.
#define TABLE_DELETE(table_ptr, key_name, key)                          \
    {                                                                   \
        int32 hash = get_hash(key, NUM_TABLE_BUCKETS);                    \
        auto current = table_ptr[hash];                                 \
        while (current) {                                               \
            if (current->key_name == key) {                             \
                if (current->table_prev) {                              \
                    current->table_prev->table_next = current->table_next; \
                } else {                                                \
                    table_ptr[hash] = current->table_next;              \
                }                                                       \
                if (current->table_next) {                              \
                    current->table_next->table_prev = current->table_prev; \
                }                                                       \
                break;                                                  \
            } else {                                                    \
                current = current->table_next;                          \
            }                                                           \
        }                                                               \
    }                                                                   \

// delete it from table, but get the result as well
#define TABLE_DELETE_GET(table_ptr, key_name, key, result)              \
    {                                                                   \
        int32 hash = get_hash(key, NUM_TABLE_BUCKETS);                  \
        result = NULL;                                                  \
        auto current = table_ptr[hash];                                 \
        while (current) {                                               \
            if (current->key_name == key) {                             \
                if (current->table_prev) {                              \
                    current->table_prev->table_next = current->table_next; \
                } else {                                                \
                    table_ptr[hash] = current->table_next;              \
                }                                                       \
                if (current->table_next) {                              \
                    current->table_next->table_prev = current->table_prev; \
                }                                                       \
                result = current;                                       \
                break;                                                  \
            } else {                                                    \
                current = current->table_next;                          \
            }                                                           \
        }                                                               \
    }

#define LINKED_LIST_FOR(entry_type, list_ptr)                           \
    for (entry_type *current = (list_ptr); current != NULL; current = current->next)

#define COMMON_H
#endif
