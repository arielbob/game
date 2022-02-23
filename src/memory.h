#ifndef MEMORY_H
#define MEMORY_H

struct Stack_Allocator {
    void *base;
    void *top;
    uint32 size;
};

struct Memory {
    bool32 is_initted;
    Stack_Allocator global_stack;
};

struct Marker {
    void *start;
};

struct Arena {
    void *base;
    uint32 size;
    uint32 used;
};

#endif
