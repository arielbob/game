#ifndef MEMORY_H
#define MEMORY_H

// we have an allocator type for read only memory where allocating is an error and deallocating is a no-op.
// we do this so we don't have to check if an allocator is null. we treat a null allocator differently from
// having a type of READ_ONLY_ALLOCATOR, since we can easily assert if an allocator is null in the case where we
// forget to set an allocator.
enum Allocator_Type {
    NONE_ALLOCATOR,
    READ_ONLY_ALLOCATOR, 
    STACK_ALLOCATOR,
    ARENA_ALLOCATOR,
    POOL_ALLOCATOR,
    HEAP_ALLOCATOR
};

struct Allocator {
    Allocator_Type type;
};

struct Read_Only_Allocator {
    Allocator_Type type;
};

Read_Only_Allocator _read_only_allocator = { READ_ONLY_ALLOCATOR };
Allocator *read_only_allocator = (Allocator *) &_read_only_allocator;

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
    uint32 blocks_used;
    uint32 max_blocks;
};

struct Heap_Block {
    Heap_Block *next;
    uint32 size;
};

struct Heap_Allocator {
    Allocator_Type type;
    void *base;
    uint32 size;
    uint32 used;
    Heap_Block *first_block;
};

struct Memory {
    bool32 is_initted;

    void *base;
    uint32 used;
    
    Stack_Allocator global_stack;
    // i'm not exactly sure why hash_table_stack is a stack; this can be an arena, but either way, it'll
    // probably be replaced
    Stack_Allocator hash_table_stack;

    Arena_Allocator game_data;
    Arena_Allocator font_arena;
    Arena_Allocator frame_arena;

    Arena_Allocator ui_arena;
    Arena_Allocator editor_arena;
};

Marker begin_region();
void end_region(Marker marker);
void *allocate(Allocator *allocator, uint32 size, bool32 zero_memory = true);
void deallocate(Allocator *allocator, void *address);

#endif
