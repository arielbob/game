#ifndef MEMORY_H
#define MEMORY_H

enum Allocator_Type {
    STACK_ALLOCATOR,
    ARENA_ALLOCATOR,
    POOL_ALLOCATOR
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

struct Pool_Allocator {
    Allocator_Type type;
    void *base;
    void *first;
    uint32 size;
    uint32 block_size;
    uint32 max_blocks;
};

struct Memory {
    bool32 is_initted;
    Arena_Allocator game_data;
    Arena_Allocator font_arena;
    Arena_Allocator mesh_arena;
    Arena_Allocator frame_arena;

    Arena_Allocator level_arena;
    Pool_Allocator level_string64_pool;
    Pool_Allocator level_filename_pool;

    Arena_Allocator string_arena; 
    Stack_Allocator global_stack;
    // i'm not exactly sure why hash_table_stack is a stack; this can be an arena, but either way, it'll
    // probably be replaced
    Stack_Allocator hash_table_stack;
    Pool_Allocator string64_pool;
    Pool_Allocator filename_pool;
};

void *allocate(Allocator *allocator, uint32 size, bool32 zero_memory = true);

#endif
