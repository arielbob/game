#ifndef MEMORY_H
#define MEMORY_H

struct Platform_Critical_Section;

// we have an allocator type for read only memory where allocating is an error and deallocating is a no-op.
// we do this so we don't have to check if an allocator is null. we treat a null allocator differently from
// having a type of READ_ONLY_ALLOCATOR, since we can easily assert if an allocator is null in the case where we
// forget to set an allocator.
enum Allocator_Type {
    NONE_ALLOCATOR,
    READ_ONLY_ALLOCATOR, 
    STACK_ALLOCATOR,
    ARENA_ALLOCATOR,
    STACK_REGION_ALLOCATOR, // regions created by a stack allocator
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

struct Stack_Allocator;

struct Stack_Region {
    Allocator_Type type;
    void *base;
    uint32 size; // will increase if at the top of the global temp allocator's stack when allocating
    uint32 used;

    Stack_Allocator *stack;
    Stack_Region *prev; // for setting stack's top_region when popping regions off
};

struct Stack_Allocator {
    Allocator_Type type;
    void *base;
    void *top;
    uint32 size;
    Stack_Region *top_region;
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
    Platform_Critical_Section critical_section;
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

    Arena_Allocator game_data;
    Arena_Allocator font_arena;
    Arena_Allocator frame_arena;

    Arena_Allocator ui_arena;
    Arena_Allocator editor_arena;
};

// allocator at the end just so that we can do begin_region(256) or whatever without
// specifying an allocator
Allocator *begin_region(Allocator *allocator = NULL, uint32 size = 0);
//Allocator *begin_region(uint32 size = 0);
void end_region(Allocator *region);
// TODO: does this need to be default true?
void *allocate(Allocator *allocator, uint32 size, bool32 zero_memory = true); 
void deallocate(Allocator *allocator, void *address);
inline void *allocate(Stack_Region *region, uint32 size, bool32 zero_memory = false);

#endif
