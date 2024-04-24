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

void *arena_push(Arena_Allocator *arena, uint32 size,
                 bool32 zero_memory = true, uint32 alignment_bytes = 8) {
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
    // the bytes we added for alignment. we do NOT want to instead subtract it, since the actual size would
    // then be smaller than size. this is not desirable since we often push structs and the size allocated
    // should be the size passed in.
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
    stack.top_region = NULL;
    return stack;
}

internal void verify(Stack_Allocator *stack) {
    assert(stack->base == stack->top);
}

#if 0
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
#endif

#if 0
Marker begin_region(Stack_Allocator *stack) {
    Marker marker;
    marker.start = stack->top;
    return marker;
}

void end_region(Stack_Allocator *stack, Marker marker) {
    assert(marker.start <= stack->top);
    stack->top = marker.start;
}
#endif

// TODO: is this still correct with the new Stack_Region stuff? i'm not sure...
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

// this is basically the same as arena_push()
void *stack_region_allocate(Stack_Region *region, uint32 size, bool32 zero_memory = false,
                            uint32 alignment_bytes = 8) {
    assert(((alignment_bytes) & (alignment_bytes - 1)) == 0); // ensure that alignment_bytes is a power of 2

    uint32 align_mask = alignment_bytes - 1;
    uint32 misalignment = ((uint64) ((uint8 *) region->base + region->used)) & align_mask;
    uint32 align_offset = 0;
    if (misalignment) {
        align_offset = alignment_bytes - misalignment;
    }
    
    size += align_offset;

    assert(region->stack);

    Stack_Allocator *stack = region->stack;
    
    if (region->used + size >= region->size) {
        if (stack->top_region == region) {
            uint32 size_delta = (region->used + size) - region->size;

            assert(((uint8 *) stack->top + size_delta) <= ((uint8 *) stack->base + stack->size));

            stack->top = (uint8 *) stack->top + size_delta;
            region->size += size_delta;
        } else {
            assert(!"Region size limit reached!");
        }
    }
    
    // get the start of the aligned allocated memory, which is just base + used + the amount of bytes we
    // added for alignment
    void *start_byte = (void *) ((uint8 *) region->base + region->used + align_offset);
    region->used += size;

    if (zero_memory) {
        platform_zero_memory(start_byte, size);
    } 

    return start_byte;
}

// TODO: this is kind of confusing because one uses the stack base/top, while the other one uses Stack_Region
// - this one is specifically for the temp_region and uses Stack_Region
inline void *region_push(uint32 size, bool32 zero_memory = true, uint32 alignment_bytes = 8) {
    if (!memory.global_stack.top_region) {
        assert(!"No temp region has began! Call begin_region() before calling region_push().");
    }
    return stack_region_allocate(memory.global_stack.top_region, size, zero_memory, alignment_bytes);
}

Allocator *begin_region(Allocator *allocator, uint32 size) {
    if (!allocator) {
        // should only do this on main thread   
        assert(memory.is_initted);
        allocator = (Allocator *) &memory.global_stack;
    }
    assert(allocator->type == STACK_ALLOCATOR);

    Stack_Allocator *stack = (Stack_Allocator *) allocator;

    void *base = stack->top;
    Stack_Region *region = (Stack_Region *) region_push(stack, sizeof(Stack_Region), false);

    uint32 info_struct_size = (uint32) ((uint8 *) stack->top - (uint8 *) base);
    size += info_struct_size;

    region->type = STACK_REGION_ALLOCATOR;
    region->base = base;
    region->size = size;
    region->used = info_struct_size;

    region->stack = stack;
    region->prev = stack->top_region;
    stack->top_region = region;
    stack->top = (uint8 *) region->base + region->size;
    
    Allocator *region_allocator = (Allocator *) region;
    return region_allocator;
}

#if 1
Allocator *begin_region(uint32 size) {
    return begin_region((Allocator *) &memory.global_stack, size);

#if 0
    void *base = memory.global_stack.top;
    Stack_Region *region = (Stack_Region *) region_push(&memory.global_stack, sizeof(Stack_Region), false);

    uint32 info_struct_size = (uint32) ((uint8 *) memory.global_stack.top - (uint8 *) base);
    size += info_struct_size;

    region->type = STACK_REGION_ALLOCATOR;
    region->base = base;
    region->size = size;
    region->used = info_struct_size;
    
    region->stack = &memory.global_stack;
    region->prev = memory.global_stack.top_region;
    memory.global_stack.top_region = region;
    memory.global_stack.top = (uint8 *) region->base + region->size;

    Allocator *region_allocator = (Allocator *) region;
    return region_allocator;
#endif
}
#endif

void end_region(Allocator *allocator) {
    assert(allocator->type == STACK_REGION_ALLOCATOR);
    Stack_Region *region = (Stack_Region *) allocator;

    if (region->stack->top_region != region) {
        assert(!"A region was not closed by end_region() or you are trying to end a stack region that was already ended. Regions should be closed in the reverse order that they are created.");
    }

    region->stack->top_region = region->prev;

    Stack_Region *top_region = region->stack->top_region;
    if (top_region) {
        region->stack->top = (uint8 *) top_region->base + top_region->used;
    } else {
        region->stack->top = memory.global_stack.base;
    }
    
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
    pool_allocator.blocks_used = 0;

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

    pool->blocks_used++;

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
    pool->blocks_used--;
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
    pool->blocks_used = 0;
}

#define HEAP_BLOCK_ALIGNMENT_BYTES 8
#define HEAP_HEADER_SIZE sizeof(uint32)
#define HEAP_HEADER_ALIGNMENT_BYTES 8
#define MIN_HEAP_SIZE sizeof(Heap_Block) + HEAP_BLOCK_ALIGNMENT_BYTES

Heap_Allocator make_heap_allocator(void *base, uint32 size) {
    // +8 for alignment
    // this ensures that we can at least store the data for a heap block, but doesn't really say anything about
    // whether we can actually store anything in it, which is fine, since we'll always allocate enough memory
    // for a large heap
    assert(size >= MIN_HEAP_SIZE);

    uint32 align_mask = HEAP_BLOCK_ALIGNMENT_BYTES - 1;
    uint32 misalignment = ((uint64) ((uint8 *) base) & align_mask);
    uint32 align_offset = HEAP_BLOCK_ALIGNMENT_BYTES - misalignment;

    base = (void *) ((uint8 *) base + align_offset);

    Heap_Allocator heap;
    heap.type = HEAP_ALLOCATOR;
    heap.base = base;
    heap.size = size;
    heap.used = 0;
    heap.critical_section = platform_make_critical_section();
    
    Heap_Block *first_block = (Heap_Block *) base;
    *((uint8 *) first_block - 1) = (uint8) align_offset;
    first_block->next = NULL;
    first_block->size = size;
    heap.first_block = first_block;
    return heap;
}

inline uint8 *heap_get_unaligned_address(void *aligned_address) {
    uint8 align_offset = *((uint8 *) aligned_address - 1);
    uint8 *unaligned_start = (uint8 *) aligned_address - align_offset;
    return unaligned_start;
}

inline uint8 *get_block_unaligned_address(void *block) {
    return heap_get_unaligned_address(block);
}

void heap_coalesce_blocks(Heap_Allocator *heap) {
    Heap_Block *block = heap->first_block;
    
    while (block != NULL) {
        Heap_Block *next_block = block->next;
        if (next_block == NULL) {
            return;
        }

        uint8 *block_start = get_block_unaligned_address(block);
        uint8 *block_end = block_start + block->size;

        uint8 *next_block_start = get_block_unaligned_address(next_block);
        uint8 *next_block_end = next_block_start + next_block->size;

        if (block_end == next_block_start) {
            block->size += next_block->size;
            block->next = next_block->next;

            assert(((uint8 *) block->next) < (heap_get_unaligned_address(heap->base) + heap->size));
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

void *heap_allocate(Heap_Allocator *heap, uint32 size, bool32 zero_memory = false, uint32 alignment_bytes = 8) {
    assert(alignment_bytes >= 4);
    assert(((alignment_bytes) & (alignment_bytes - 1)) == 0); // ensure that alignment_bytes is a power of 2

    void *start_byte = NULL;
    Heap_Block *block = heap->first_block;
    Heap_Block *previous_block = NULL;
    while (block != NULL) {
        if (block->size < size) {
            previous_block = block;
            block = block->next;
            continue;
        }

        // we save the header size after the alignment padding. this is because we don't want to cause
        // an unaligned access, although this does cause alignment_bytes to have to be >= 4, since
        // header_size is sizeof(uint32) = 4. we don't have to do this to access the offset, because
        // offset is stored in a single byte, and any address % 1 = 0, so that won't cause unaligned access.

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

        uint32 aligned_size = 0;
        uint8 *unaligned_start = get_block_unaligned_address(block);
        uint8 *current_address = unaligned_start;

        // align the header
        uint32 header_aligned_size, header_align_offset;
        void *header_address = get_aligned_address(current_address, HEAP_HEADER_SIZE,
                                                   &header_aligned_size, &header_align_offset);
        aligned_size += header_aligned_size;
        current_address += header_aligned_size;
        
        // align the data
        uint32 data_aligned_size, data_align_offset;
        void *data_address = get_aligned_address(current_address, size,
                                                 &data_aligned_size, &data_align_offset);
        aligned_size += data_aligned_size;
        current_address += data_aligned_size;

        // when we deallocate this block, we need to put a Heap_Block struct here, so we make sure
        // that we allocate enough space to do that.
        uint32 free_block_aligned_size, free_block_align_offset;
        void *free_block_address = get_aligned_address(unaligned_start, sizeof(Heap_Block),
                                                       &free_block_aligned_size, &free_block_align_offset);
        aligned_size = max(aligned_size, free_block_aligned_size);

        start_byte = data_address;

        if (block->size < aligned_size) {
            previous_block = block;
            block = block->next;
            continue;
        }

        uint32 remaining_bytes_in_block = block->size - aligned_size;

        void *allocated_end = (uint8 *) unaligned_start + aligned_size;
        uint32 new_block_aligned_size, new_block_offset;
        void *aligned_block_address = get_aligned_address(allocated_end, sizeof(Heap_Block),
                                                          &new_block_aligned_size, &new_block_offset);

        Heap_Block *next_block = NULL;
        if (remaining_bytes_in_block < new_block_aligned_size) {
            // if we can't get another block out of the remaining bytes of the block we're using, then
            // just absorb that extra space, since we would have no way of keeping track of all those tiny
            // regions (since they wouldn't be included in the free list, since we can't fit a Heap_Block in them)
            aligned_size += remaining_bytes_in_block;
            allocated_end = (uint8 *) unaligned_start + aligned_size;
            
            next_block = block->next;
        } else {
            // store a new block
            next_block = (Heap_Block *) aligned_block_address;
            *((uint8 *) aligned_block_address - 1) = (uint8) new_block_offset;
            
            next_block->size = remaining_bytes_in_block;
            next_block->next = block->next;
        }
        
        // store the align offset a single byte before the returned memory address, so that when we deallocate
        // we can just look one byte before to see how far we need to go backwards to find the beginning of
        // the block
        *((uint8 *) data_address - 1) = (uint8) data_align_offset;
        *((uint32 *) (header_address)) = aligned_size;
        *((uint8 *) ((uint8 *) header_address - 1)) = (uint8) header_align_offset;

        if (zero_memory) {
            platform_zero_memory(data_address, size);
        }

        if (previous_block == NULL) {
            heap->first_block = next_block;
        } else {
            previous_block->next = next_block;
        }

        heap->used += aligned_size;
        assert(heap->used < heap->size);

        break;
    }

    if (block == NULL) {
        assert(!"Could not find free block.");

        return NULL;
    }

    assert(start_byte < (heap_get_unaligned_address(heap->base) + heap->size));

    return start_byte;
}

void heap_deallocate(Heap_Allocator *heap, void *address) {
    // get header
    uint8 data_align_offset = *((uint8 *) address - 1);
    uint8 *header_address = (uint8 *) address - data_align_offset - HEAP_HEADER_SIZE;
    uint32 size = *((uint32 *) header_address);

    // get unaligned start of block
    uint8 header_align_offset = *(header_address - 1);
    uint8 *unaligned_block_address = header_address - header_align_offset;
    
    uint32 aligned_free_block_size, free_block_align_offset;
    Heap_Block *free_block = (Heap_Block *) get_aligned_address(unaligned_block_address, sizeof(Heap_Block),
                                                                &aligned_free_block_size, &free_block_align_offset);
    free_block->size = size;

    assert(heap->used >= size);
    heap->used -= size;

    // make sure that the size of the block metadata struct is less or equal to the size we just deallocated
    assert(aligned_free_block_size <= size);

    // store the block metadata alignment
    *((uint8 *) free_block - 1) = (uint8) free_block_align_offset;

    Heap_Block *block = heap->first_block;
    // insert the block such that the free list is in ascending order.
    // this makes coalescing much simpler.
    if (block == NULL) {
        free_block->next = NULL;
        heap->first_block = free_block;

        assert(free_block == heap->base);

    } else if (block > free_block) {
        free_block->next = block;
        heap->first_block = free_block;
    } else {
        while (block != NULL) {
            if (block->next == NULL || block->next > free_block) {
                free_block->next = block->next;
                block->next = free_block;
                break;
            } else {
                block = block->next;
            }
        }
    }

    assert(heap->used < heap->size);
    assert(free_block != free_block->next);
    
    // debug
#if 0
    block = heap->first_block;
    while (block != NULL) {
        assert(block->size >= 0);
        block = block->next;
    }
#endif

    heap_coalesce_blocks(heap);
}

void clear_heap(Heap_Allocator *heap) {
    void *unaligned_base = heap_get_unaligned_address(heap->base);

    uint32 align_mask = HEAP_BLOCK_ALIGNMENT_BYTES - 1;
    uint32 misalignment = ((uint64) ((uint8 *) unaligned_base) & align_mask);
    uint32 align_offset = HEAP_BLOCK_ALIGNMENT_BYTES - misalignment;

    void *base = (void *) ((uint8 *) unaligned_base + align_offset);

    heap->used = 0;

    assert(base == heap->base);
    Heap_Block *first_block = (Heap_Block *) base;
    *((uint8 *) first_block - 1) = (uint8) align_offset;
    first_block->next = NULL;
    first_block->size = heap->size;
    heap->first_block = first_block;
}

inline void *allocate(Arena_Allocator *arena, uint32 size, bool32 zero_memory = false) {
    return arena_push(arena, size, zero_memory);
}

inline void *allocate(Heap_Allocator *heap, uint32 size, bool32 zero_memory = false) {
    return heap_allocate(heap, size, zero_memory);
}

inline void *allocate(Stack_Region *region, uint32 size, bool32 zero_memory) {
    return stack_region_allocate(region, size, zero_memory);
}

// TODO: should have alignment parameter too, i think
void *allocate(Allocator *allocator, uint32 size, bool32 zero_memory) {
    switch (allocator->type) {
        case READ_ONLY_ALLOCATOR: {
            assert(!"Cannot allocate to read-only memory.");
            return NULL;
        }
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
        case HEAP_ALLOCATOR:
        {
            Heap_Allocator *heap = (Heap_Allocator *) allocator;
            return heap_allocate(heap, size, zero_memory);
        }
        case STACK_REGION_ALLOCATOR:
        {
            Stack_Region *region = (Stack_Region *) allocator;
            return stack_region_allocate(region, size, zero_memory);
        }
        default:
        {
            assert(!"Unhandled allocator type");
        } break;
    }

    return NULL;
}

void deallocate(Allocator *allocator, void *address) {
    assert(address);
    
    switch (allocator->type) {
        case READ_ONLY_ALLOCATOR: {
            // no-op
            return;
        }
        case STACK_ALLOCATOR: {
            assert(!"Stacks are cleared using end_region()");
            return;
        }
        case ARENA_ALLOCATOR: {
            assert(!"Arenas are cleared all at once using clear_arena()");
            return;
        }
        case POOL_ALLOCATOR: {
            Pool_Allocator *pool = (Pool_Allocator *) allocator;
            return pool_remove(pool, address);
        }
        case HEAP_ALLOCATOR: {
            Heap_Allocator *heap = (Heap_Allocator *) allocator;
            return heap_deallocate(heap, address);
        }
        default: {
            assert(!"Unhandled allocator type.");
            return;
        } break;
    }
}

void clear(Allocator *allocator, bool32 zero_memory = false) {
    switch (allocator->type) {
        case STACK_ALLOCATOR: {
            // TODO: implement a clear stack procedure?
            assert(!"Stacks are cleared using end_region()");
            return;
        }
        case ARENA_ALLOCATOR: {
            Arena_Allocator *arena = (Arena_Allocator *) allocator;
            clear_arena(arena, zero_memory);
            return;
        }
        case POOL_ALLOCATOR: {
            Pool_Allocator *pool = (Pool_Allocator *) allocator;
            clear_pool(pool);
            return;
        }
        case HEAP_ALLOCATOR: {
            Heap_Allocator *heap = (Heap_Allocator *) allocator;
            clear_heap(heap);
            return;
        }
        default: {
            assert(!"Unhandled allocator type.");
            return;
        } break;
    }
}

//#define copy_struct(allocator_pointer, type, value) *(allocate(allocator_pointer, sizeof(type))) = value

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

uint32 get_remaining(Allocator *allocator) {
    switch (allocator->type) {
        case STACK_ALLOCATOR: {
            Stack_Allocator *stack = (Stack_Allocator *) allocator;
            uint32 used = (uint32) ((uint8 *) stack->top - (uint8 *) stack->base);
            return stack->size - used;
        }
        case ARENA_ALLOCATOR: {
            Arena_Allocator *arena = (Arena_Allocator *) allocator;
            return arena->size - arena->used;
        }
        case POOL_ALLOCATOR: {
            Pool_Allocator *pool = (Pool_Allocator *) allocator;
            uint32 used = pool->blocks_used * pool->block_size;
            return pool->size - used;
        }
        case HEAP_ALLOCATOR: {
            Heap_Allocator *heap = (Heap_Allocator *) allocator;
            return heap->size - heap->used;
        }
        case STACK_REGION_ALLOCATOR: {
            Stack_Region *region = (Stack_Region *) allocator;
            return region->size - region->used;
        }
        default: {
            assert(!"Unhandled allocator type.");
            return 0;
        } break;
    }
}
