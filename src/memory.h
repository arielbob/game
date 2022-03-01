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

struct Marker {
    void *start;
};

struct Arena_Allocator {
    Allocator_Type type;
    void *base;
    uint32 size;
    uint32 used;
};

struct Memory {
    bool32 is_initted;
    // Arena_Allocator game_data;
    Stack_Allocator global_stack;
    Stack_Allocator hash_table_stack;
};

#endif
