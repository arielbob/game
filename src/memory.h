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
    Arena_Allocator game_data;
    Arena_Allocator font_arena;
    Arena_Allocator mesh_arena;
    // TODO: we may want to have a free-list version of this for strings that are always the same size,
    //       such as for filepaths
    Arena_Allocator string_arena; 
    Stack_Allocator global_stack;
    Stack_Allocator hash_table_stack;
};

#endif
