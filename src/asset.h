#ifndef ASSET_H
#define ASSET_H

#include "mesh.h"

#define MAX_MESHES 64
#define MAX_ASSET_STRING_SIZE 64

struct Asset_Manager {
    Arena_Allocator *persistent_mesh_arena_pointer;
    Arena_Allocator *persistent_string_arena_pointer;

    Heap_Allocator *level_mesh_heap_pointer;

    Pool_Allocator *string_pool_pointer;   // pool with block size of MAX_ASSET_STRING_SIZE
    Pool_Allocator *filename_pool_pointer; // pool with block size of PLATFORM_MAX_PATH

    Hash_Table<int32, Mesh> mesh_table;
};

#endif
