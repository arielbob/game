#include "memory.h"

Arena_Allocator make_arena_allocator(void *base, uint32 size) {
    Arena_Allocator arena;
    arena.type = ARENA_ALLOCATOR;
    arena.base = base;
    arena.size = size;
    arena.used = 0;
    return arena;
}

void *arena_push(Arena_Allocator *arena, uint32 size, bool32 zero_memory = true) {
    assert((arena->used + size) <= arena->size);

    void *start_byte = (void *) (((uint8 *) arena->base) + arena->used);
    arena->used += size;

    if (zero_memory) {
        platform_zero_memory(start_byte, size);
    } 
   
    return start_byte;
}

void clear_arena(Arena_Allocator *arena, bool32 zero_memory = false) {
    arena->used = 0;
    // NOTE: zero_memory is false by default, since if the arena has a lot of used memory, the zeroing
    //       procedure can be slow.
    if (zero_memory) {
        platform_zero_memory(arena->base, arena->size);
    }
}

Stack_Allocator make_stack_allocator(void *base, uint32 size) {
    Stack_Allocator stack;
    stack.type = STACK_ALLOCATOR;
    stack.base = base;
    stack.top = base;
    stack.size = size;
    return stack;
}

internal void verify(Stack_Allocator *stack) {
    assert(stack->base == stack->top);
}

Marker begin_region(Memory *memory) {
    assert(memory->is_initted);

    Marker marker;
    marker.start = memory->global_stack.top;
    return marker;
}

void end_region(Memory *memory, Marker marker) {
    assert(marker.start <= memory->global_stack.top);
    memory->global_stack.top = marker.start;
}

void *region_push(Memory *memory, uint32 size, bool32 zero_memory = true) {
    Stack_Allocator *stack = &memory->global_stack;
    assert(((uint8 *) stack->top + size) <= ((uint8 *) stack->base + stack->size));

    if (zero_memory) {
        platform_zero_memory(stack->top, size);
    }
    
    void *base = stack->top;
    stack->top = (uint8 *) stack->top + size;

    return base;
}

Marker begin_region(Stack_Allocator *stack) {
    Marker marker;
    marker.start = stack->top;
    return marker;
}

void end_region(Stack_Allocator *stack, Marker marker) {
    assert(marker.start <= stack->top);
    stack->top = marker.start;
}

void *region_push(Stack_Allocator *stack, uint32 size, bool32 zero_memory = true) {
    assert(((uint8 *) stack->top + size) <= ((uint8 *) stack->base + stack->size));

    if (zero_memory) {
        platform_zero_memory(stack->top, size);
    }
    
    void *base = stack->top;
    stack->top = (uint8 *) stack->top + size;

    return base;
}

void *allocate(Allocator *allocator, uint32 size, bool32 zero_memory = true) {
    switch (allocator->type) {
        case STACK_ALLOCATOR:
        {
            Stack_Allocator *stack = (Stack_Allocator *) allocator;
            return region_push(stack, size, zero_memory);
        } break;
        case ARENA_ALLOCATOR:
        {
            Arena_Allocator *arena = (Arena_Allocator *) allocator;
            return arena_push(arena, size, zero_memory);
        }
        default:
        {
            assert(false);
        } break;
    }

    return NULL;
}

// #define allocate(allocator, size) _allocate((Allocator *) allocator, size)
// #define allocate(allocator, size, zero_memory) _allocate((Allocator *) allocator, size, zero_memory)

#if 0
void *arena_alloc(Arena *arena, uint32 size_to_allocate) {
    assert((arena->used + size_to_allocate) <= arena->size);

    void *start_byte = (void *) (((uint8 *) arena->base) + arena->used);
    arena->used += size_to_allocate;

    return start_byte;
}
#endif
