#ifndef MEMORY_H
#define MEMORY_H

enum Allocator_Type {
    STACK_ALLOCATOR,
    ARENA_ALLOCATOR
};

struct Allocator {
    Allocator_Type type;
};

struct Stack_Allocator {
    Allocator_Type type;
    void *base;
    void *top;
    uint32 size;
};

struct Memory {
    bool32 is_initted;
    Stack_Allocator global_stack;
    Stack_Allocator hash_table_stack;
};

struct Marker {
    void *start;
};

struct Arena {
    Allocator_Type type;
    void *base;
    uint32 size;
    uint32 used;
};

#endif
