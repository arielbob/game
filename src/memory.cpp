#include "memory.h"

// as long as you aren't using data types that are larger than 8 bytes, alignment_bytes=8 will accomodate every type.
// even for structs, structs just need to be aligned to the largest member, so as long as we're not using data types
// larger than 8 bytes, then an alignment of 8 will be fine.
inline void *get_aligned_address(void *base, uint32 size,
                                 uint32 *aligned_size_result, uint32 *align_offset_result, uint32 alignment_bytes=8) {
    assert(((alignment_bytes) & (alignment_bytes - 1)) == 0);
    uint32 align_mask = alignment_bytes - 1;
    uint32 misalignment = ((uint64) (base)) & align_mask;
    uint32 align_offset = alignment_bytes - misalignment;

    *aligned_size_result = align_offset + size;
    *align_offset_result = align_offset;
    return ((uint8 *) base + align_offset);
}

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
    assert(block_size >= sizeof(void *));
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
    // NOTE: we don't have to worry about whether or not block_address is the address of the data (i.e. no
    //       padding, or the start of the block (with padding), since pool entries are a constant size,
    //       contiguous, and of a power of 2 size, so as long as the first one is aligned, the rest will also
    //       be aligned. (see also the NOTE above make_pool_allocator())
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

Heap_Allocator make_heap_allocator(void *base, uint32 size, bool32 zero_memory = true, uint32 alignment_bytes = 8) {
    assert(((alignment_bytes) & (alignment_bytes - 1)) == 0); // ensure that alignment_bytes is a power of 2

    // +8 for alignment
    assert(size >= sizeof(Heap_Block) + 8);

    uint32 align_mask = alignment_bytes - 1;
    uint32 misalignment = ((uint64) ((uint8 *) base) & align_mask);
    uint32 align_offset = align_offset = alignment_bytes - misalignment;

    //size -= align_offset;
    base = (void *) ((uint8 *) base + align_offset);

    Heap_Allocator heap;
    heap.type = HEAP_ALLOCATOR;
    heap.base = base;
    heap.size = size;
    //heap.used = 0;
    
    Heap_Block *first_block = (Heap_Block *) base;
    *((uint8 *) first_block - 1) = (uint8) align_offset;
    first_block->next = NULL;
    first_block->size = size;
    heap.first_block = first_block;
    //heap.end = (uint8 *) heap.base + size;
    return heap;
}

#if 0
void heap_sort_block_free_list(Heap_Allocator *heap) {
    Heap_Block *sorted_head = heap->first_block;

    Heap_Block *block = sorted_head;
    while (block) {
        Heap_Block *sorted_block = sorted_head;
        while (sorted_block) {
            if (sorted_block->next == NULL || sorted_block->next > block) {
                block->next = sorted_block->next;
                sorted_block->next = block;
                // skip over the block we just added, i.e. go to the original sorted_block->next
                block = block->next;
            }
        }
    }
}
#endif

inline uint8 *get_block_unaligned_address(void *block) {
    uint8 block_offset = *((uint8 *) block - 1);
    uint8 *block_start = (uint8 *) block - block_offset;
    return block_start;
}

void heap_coalesce_blocks(Heap_Allocator *heap) {
    Heap_Block *block = heap->first_block;
    
    while (block != NULL) {
        Heap_Block *next_block = block->next;
        if (next_block == NULL) return;

        uint8 *block_start = get_block_unaligned_address(block);
        uint8 *block_end = block_start + block->size;

        uint8 *next_block_start = get_block_unaligned_address(next_block);
        uint8 *next_block_end = next_block_start + next_block->size;

        if (block_end == next_block_start) {
            block->size += next_block->size;
            block->next = next_block->next;
            // it is not an error that we don't set block = block->next if we coalesced.
            // if we have multiple contiguous free blocks, for example 3, if we coalesce the first 2,
            // and then just set block = block->next, we would go to the third block, then check if
            // third_block + size == third_block->end, but we wouldn't coalesce the 1+2 block with
            // the 3 block. so instead, we stay on the 1+2 block, checking if it has anything to
            // coalesce with, until it doesn't. at that point we follow the next pointer.
        } else {
            block = block->next;
        }
    }
}

#define HEAP_HEADER_SIZE sizeof(uint32)

void *heap_allocate(Heap_Allocator *heap, uint32 size, uint32 alignment_bytes = 8) {
    assert(alignment_bytes >= 4);
    assert(((alignment_bytes) & (alignment_bytes - 1)) == 0); // ensure that alignment_bytes is a power of 2

    void *start_byte = NULL;
    Heap_Block *block = heap->first_block;
    Heap_Block *previous_block = NULL;
    while (block != NULL) {
        if (block->size < size) {
            block = block->next;
            previous_block = block;
            continue;
        }

        // we save the header size after the alignment padding. this is because we don't want to cause
        // an unaligned access, although this does cause alignment_bytes to have to be >= 4, since
        // header_size is sizeof(uint32) = 4. we don't have to do this to access the offset, because
        // offset is stored in a single byte, and any address % 1 = 0, so that won't cause unaligned access.
        uint32 aligned_size = size;

        uint8 block_offset = *((uint8 *) block - 1);
        uint8 *unaligned_start = (uint8 *) block - block_offset;
        uint8 *current_address = unaligned_start;

        /*
          [padding (header_align_offset bytes)] [size header (4 bytes)] [padding (data_align_offset bytes)] [data]

          to access the padding, which is stored in the last byte of the padding, you would do
          uint8 align_offset = *(data - 1).
          to access the header, you would do *(data - align_offset - HEAP_HEADER_SIZE).
          header_align_offset = *(data - align_offset - HEAP_HEADER_SIZE - 1).
          to access the block, you would do *(data - align_offset - HEAP_HEADER_SIZE - header_align_offset).

          we could have done this by just making the header size 8 bytes, then adding that onto the data size, and
          since the header size is 8, the offset data would already be aligned. but, that is not the case if the
          alignment is larger than 8. so we do it this way instead, so we don't have to write another procedure
          if we want an alignment larger than 8 bytes.
         */

        // align the header
        uint32 align_mask = HEAP_HEADER_SIZE - 1; // align_mask should be less than 8, i think?
        uint32 misalignment = ((uint64) current_address) & align_mask;
        uint32 header_align_offset = HEAP_HEADER_SIZE - misalignment;
        aligned_size += header_align_offset + HEAP_HEADER_SIZE;
        current_address += header_align_offset + HEAP_HEADER_SIZE;
        
        // align the data
        align_mask = alignment_bytes - 1;
        misalignment = ((uint64) (current_address)) & align_mask;
        uint32 data_align_offset = alignment_bytes - misalignment;
        aligned_size += data_align_offset;
        current_address += data_align_offset;

        start_byte = current_address;

        if (block->size < aligned_size) {
            block = block->next;
            previous_block = block;
            continue;
        }

        uint32 remaining_bytes_in_block = block->size - aligned_size;

        void *allocated_end = (uint8 *) unaligned_start + aligned_size;
        uint32 new_block_aligned_size, new_block_offset;
        void *aligned_block_address = get_aligned_address(allocated_end, sizeof(Heap_Block),
                                                          &new_block_aligned_size, &new_block_offset);

        if (remaining_bytes_in_block < new_block_aligned_size) {
            // if we can't get another block out of the remaining bytes of the block we're using, then
            // just absorb that extra space, since we would have no way of keeping track of all those tiny
            // regions (since they wouldn't be included in the free list, since we can't fit a Heap_Block in them)
            aligned_size += remaining_bytes_in_block;
            allocated_end = (uint8 *) unaligned_start + aligned_size;
        }
        
        Heap_Block *original_block_next = block->next;

        // store the align offset a single byte before the returned memory address, so that when we deallocate
        // we can just look one byte before to see how far we need to go backwards to find the beginning of
        // the block
        *((uint8 *) start_byte - 1) = (uint8) data_align_offset;
        *((uint32 *) ((uint8 *) start_byte - data_align_offset - HEAP_HEADER_SIZE)) = aligned_size;
        *((uint8 *) ((uint8 *) start_byte - data_align_offset - HEAP_HEADER_SIZE - 1)) = (uint8) header_align_offset;

        if (remaining_bytes_in_block >= new_block_aligned_size) {
            // store a new block
            Heap_Block *next_block = (Heap_Block *) aligned_block_address;
            *((uint8 *) aligned_block_address - 1) = (uint8) new_block_offset;
            
            next_block->size = remaining_bytes_in_block;
            next_block->next = original_block_next;

            if (previous_block == NULL) {
                heap->first_block = next_block;
            } else {
                previous_block->next = next_block;
            }
        } else {
            if (previous_block == NULL) {
                heap->first_block = original_block_next;
            } else {
                previous_block->next = original_block_next;
            }
        }

        break;
    }

    if (block == NULL) {
        assert(!"Could not find free block.");
        return NULL;
    }

    return start_byte;
}

void heap_deallocate(Heap_Allocator *heap, void *address) {
    uint8 align_offset = *((uint8 *) address - 1);
    uint8 *header_address = (uint8 *) address - align_offset - HEAP_HEADER_SIZE;
    uint32 size = *((uint32 *) header_address);
    uint8 header_align_offset = *(header_address - 1);
    uint8 *unaligned_block_address = header_address - header_align_offset;
    
    uint32 aligned_block_size, block_align_offset;
    Heap_Block *block_to_add = (Heap_Block *) get_aligned_address(unaligned_block_address, sizeof(Heap_Block),
                                                                  &aligned_block_size, &block_align_offset);
    block_to_add->size = size;
    *((uint8 *) block_to_add - 1) = (uint8) block_align_offset;

    Heap_Block *block = heap->first_block;
    // insert the block such that the free list is in ascending order.
    // this makes coalescing much simpler.
    if (block == NULL) {
        block_to_add->next = NULL;
        heap->first_block = block_to_add;
    } else if (block > block_to_add) {
        block_to_add->next = block;
        heap->first_block = block_to_add;
    } else {
        while (block != NULL) {
            if (block->next == NULL || block->next > block_to_add) {
                block_to_add->next = block->next;
                block->next = block_to_add;
                break;
            } else {
                block = block->next;
            }
        }
    }

    heap_coalesce_blocks(heap);
}

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

void deallocate(Allocator *allocator, void *address) {
    switch (allocator->type) {
        case STACK_ALLOCATOR: {
            assert(!"Stacks are cleared using end_region()");
            return;
        } break;
        case ARENA_ALLOCATOR: {
            assert(!"Arenas are cleared all at once using clear_arena()");
            return;
        } break; 
        case POOL_ALLOCATOR: {
            Pool_Allocator *pool = (Pool_Allocator *) allocator;
            return pool_remove(pool, address);
        }
        default: {
            assert(!"Unhandled allocator type.");
            return;
        } break;
    }
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
