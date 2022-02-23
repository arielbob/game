#include "memory.h"

Stack_Allocator make_stack_allocator(void *base, uint32 size) {
    Stack_Allocator stack;
    stack.base = base;
    stack.top = base;
    stack.size = size;
    return stack;
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

Arena region_push(Memory *memory, uint32 size, bool32 zero_memory = true) {
    Stack_Allocator *stack = &memory->global_stack;
    assert(((uint8 *) stack->top + size) <= ((uint8 *) stack->base + stack->size));

    if (zero_memory) {
        platform_zero_memory(stack->top, size);
    }
    
    Arena arena = {};
    arena.base = stack->top;
    arena.size = size;
    stack->top = (uint8 *) stack->top + size;

    return arena;    
}

void *arena_alloc(Arena *arena, uint32 size_to_allocate) {
    assert((arena->used + size_to_allocate) <= arena->size);

    void *start_byte = (void *) (((uint8 *) arena->base) + arena->used);
    arena->used += size_to_allocate;

    return start_byte;
}

internal void verify(Stack_Allocator *stack) {
    assert(stack->base == stack->top);
}

/*

you have some memory
we want some memory allocated on the heap, that we can free afterwards
we just need it for a certain time

we call begin_region()
then whenever we need memory we call region_push(size) and we get an arena of that size
what's the difference between doing that and just creating an arena?
well, we can always create an arena, but creating an arena requires somewhere to store it
so we're just creating an arena on a stack, such that we can easily pop it off
and we can create arenas using different types of allocators

we can't really do anything with arenas. arenas are just memory. they're basically just buffers like how you
pass pointers to procedures that store the result of the procedure. the only difference is that since it's not
just a pointer, we can store extra information with the pointer such has how much space is there, so we don't
end up overflowing the buffer.

 */
