#include "memory.h"

Stack_Allocator make_stack_allocator(void *base, uint32 size) {
    Stack_Allocator stack;
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

#if 0
void *arena_alloc(Arena *arena, uint32 size_to_allocate) {
    assert((arena->used + size_to_allocate) <= arena->size);

    void *start_byte = (void *) (((uint8 *) arena->base) + arena->used);
    arena->used += size_to_allocate;

    return start_byte;
}
#endif
