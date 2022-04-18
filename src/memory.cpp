#include "memory.h"

Arena_Allocator make_arena_allocator(void *base, uint32 size) {
    Arena_Allocator arena;
    arena.type = ARENA_ALLOCATOR;
    arena.base = base;
    arena.size = size;
    arena.used = 0;
    return arena;
}

/*
  by default, align by 8 bytes (64 bits); this accomodates all <= 64 bit values
  alignment makes it so the base address of the memory you allocated is a multiple of the alignment value.
  "When accessing N bytes of memory, the base memory address must be evenly divisible by N, i.e. addr % N == 0"
  (https://www.kernel.org/doc/Documentation/unaligned-memory-access.txt)
  8 bytes (64 bits) is divisible by all the powers of 2 below and equal to it, so it works well for all
  types of data of size <= 8 bytes.

  struct My_Struct {
      int32 a;
      int32 c;
      int16 b;
      (+16 bits of padding)
  };

  16 bits (4 bytes) of padding are added to the end of My_Struct. what would happen if we didn't add
  those 4 bytes at the end? imagine we allocate an array of My_Struct's starting at 0x00. the first
  element would be at 0x00, the next would be at 0x0A (dec 10). is it possible to do an aligned access of
  array[1].a? is 10 % 4 == 0? no, so that would be an unaligned access.

  let's put the padding on now. sizeof(My_Struct) is now 12. the first element is at 0x00, the second
  element is now at 0x0C (12). let's try accessing array[1].a again. is 12 % 4 == 0? yes it is, so that
  would be an aligned access.

  arena_push(arena, sizeof(My_Struct) * 5);

  5 * 12 = 60. we would add 4 extra bytes to the end.
  60 % 7 = 4 (7 because we do alignment_bytes - 1)
  done in code: (0011 1100) & (0000 0111) = (0000 0100) = 4 bytes

 */

void *arena_push(Arena_Allocator *arena, uint32 size, bool32 zero_memory = true, uint32 alignment_bytes = 8) {
    assert(((alignment_bytes) & (alignment_bytes - 1)) == 0); // ensure that alignment_bytes is a power of 2

    uint32 align_mask = alignment_bytes - 1;
    // see how far away from an alignment boundary we are
    // [xxxx|x___|____] <- imagine that's our memory and we wan to align to 4 bytes;
    //                     misalignment would = 1. we calculate align_offset by doing 4 - 1 = 3.
    uint32 misalignment = ((uint64) ((uint8 *) arena->base + arena->used)) & align_mask;
    uint32 align_offset = 0;
    if (misalignment) {
        // note that since misalignment could be 0, we don't want to do 8 - 0 = 8 = align_offset,
        // since that would add 8 unnecessary bytes
        align_offset = alignment_bytes - misalignment;
    }
    
    // we add align_offset to size, since we need to move our arena->used variable by the size +
    // the bytes we added for alignment.
    size += align_offset;
    assert((arena->used + size) <= arena->size);

    // get the start of the aligned allocated memory, which is just base + used + the amount of bytes we
    // added for alignment
    void *start_byte = (void *) ((uint8 *) arena->base + arena->used + align_offset);
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

Marker begin_region() {
    assert(memory.is_initted);

    Marker marker;
    marker.start = memory.global_stack.top;
    return marker;
}

void end_region(Marker marker) {
    assert(marker.start <= memory.global_stack.top);
    memory.global_stack.top = marker.start;
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

void *region_push(Stack_Allocator *stack, uint32 size, bool32 zero_memory = true, uint32 alignment_bytes = 8) {
    assert(((alignment_bytes) & (alignment_bytes - 1)) == 0);

    uint32 align_mask = alignment_bytes - 1;
    uint32 misalignment = ((uint64) stack->top) & align_mask;
    uint32 align_offset = 0;
    if (misalignment) {
        align_offset = alignment_bytes - misalignment;
    }
    
    size += align_offset;
    assert(((uint8 *) stack->top + size) <= ((uint8 *) stack->base + stack->size));

    void *start_byte = (void *) ((uint8 *) stack->top + align_offset);
    stack->top = ((uint8 *) stack->top) + size;

    if (zero_memory) {
        platform_zero_memory(start_byte, size);
    }
    
    return start_byte;
}

inline void *region_push(uint32 size, bool32 zero_memory = true, uint32 alignment_bytes = 8) {
    return region_push(&memory.global_stack, size, zero_memory, alignment_bytes);
}

// NOTE: block_size is assumed to be obtained by using sizeof(some struct), so we don't have to worry
//       about struct packing/alignment.
Pool_Allocator make_pool_allocator(void *base, uint32 block_size, uint32 size, uint32 alignment_bytes = 8) {
    assert (block_size >= sizeof(void *));
    assert(((alignment_bytes) & (alignment_bytes - 1)) == 0); // ensure that alignment_bytes is a power of 2

    uint32 align_mask = alignment_bytes - 1;
    uint32 misalignment = ((uint64) ((uint8 *) base) & align_mask);
    uint32 align_offset = 0;
    if (misalignment) {
        align_offset = alignment_bytes - misalignment;
    }
    
    // we're only aligning the base here, since allocations to the pool allocator are always aligned as long as
    // you only store one type of struct/type in it.

    // since the size parameter is actually the maximum size, if base is not aligned, we will just subtract
    // align_offset from the maximum size and add the offset to the base.

    // [xxxx|x___|____]
    // assuming base is right where the x's end, align offset would be 3 here, so we subtract 3 from the
    // max size and add 3 to base.
    size -= align_offset;

    base = (void *) ((uint8 *) base + align_offset);

    Pool_Allocator pool_allocator;
    pool_allocator.type = POOL_ALLOCATOR;
    pool_allocator.size = size;
    pool_allocator.block_size = block_size;
    pool_allocator.base = base;
    pool_allocator.first = base;
    pool_allocator.max_blocks = size / block_size;

    // initialize the free list
    void **current = (void **) pool_allocator.base;
    for (uint32 i = 0; i < pool_allocator.max_blocks - 1; i++) {
        *current = current + block_size;
        current = (void **) ((uint8 *) current + block_size);
    }

    *current = NULL;

    return pool_allocator;
}

#if 0
// TODO: this procedure may be broken; we initialize the free list like we do in the other make_pool_allocator
//       procedure.
Pool_Allocator make_pool_allocator(Allocator *allocator,
                                   uint32 block_size, uint32 max_blocks) {
    uint32 pool_size_bytes = block_size * max_blocks;
    void *base = allocate(allocator, pool_size_bytes);

    Pool_Allocator pool_allocator;
    pool_allocator.type = POOL_ALLOCATOR;
    pool_allocator.size = pool_size_bytes;
    pool_allocator.block_size = block_size;
    pool_allocator.base = base;
    pool_allocator.first = base;
    pool_allocator.max_blocks = max_blocks;

    // initialize the free list
    void **current = (void **) pool_allocator.base;
    while ((uint8 *) current < (((uint8 *) base + pool_size_bytes) - block_size)) {
        // store a pointer to the next free block, which initially is just a pointer to the next block in memory
        *current = current + block_size;
        current = (void **) ((uint8 *) current +  block_size);
    }

    *current = NULL;

    return pool_allocator;
}
#endif

void *pool_push(Pool_Allocator *pool, uint32 size, bool32 zero_memory = false) {
    assert(size <= pool->block_size);

    // set the address to the next free block (which is the address stored at pool->first)
    void *base = pool->first;
    if (!base) {
        assert(!"Pool is full.");
    }

    pool->first = *((void **)pool->first);

    if (zero_memory) {
        platform_zero_memory(base, pool->block_size);
    }

    return base;
}

void pool_remove(Pool_Allocator *pool, void *block_address) {
    // the removed block becomes the first to be returned on the next allocation and the old first
    // is stored in the removed block.
    void **block_pointer = (void **) block_address;
    *block_pointer = pool->first;
    pool->first = block_pointer;
}

void clear_pool(Pool_Allocator *pool) {
    pool->first = pool->base;


    // initialize the free list
    void **current = (void **) pool->base;
    for (uint32 i = 0; i < pool->max_blocks - 1; i++) {
        *current = current + pool->block_size;
        current = (void **) ((uint8 *) current + pool->block_size);
    }
    
    *current = NULL;

}

#if 0
void *get_block(Pool_Allocator *pool, uint32 index) {
    assert(index < pool
    return pool->base + (pool->block_size * index);
}
#endif

void *allocate(Allocator *allocator, uint32 size, bool32 zero_memory) {
    switch (allocator->type) {
        case STACK_ALLOCATOR:
        {
            Stack_Allocator *stack = (Stack_Allocator *) allocator;
            return region_push(stack, size, zero_memory);
        }
        case ARENA_ALLOCATOR:
        {
            Arena_Allocator *arena = (Arena_Allocator *) allocator;
            return arena_push(arena, size, zero_memory);
        }
        case POOL_ALLOCATOR:
        {
            Pool_Allocator *pool = (Pool_Allocator *) allocator;
            return pool_push(pool, size, zero_memory);
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
