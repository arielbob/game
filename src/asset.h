#ifndef ASSET_H
#define ASSET_H

#include "mesh.h"

#define MAX_MESHES 64
#define MAX_ASSET_STRING_SIZE 64

struct Asset_Manager {
    Allocator* allocator;

    Arena_Allocator *persistent_mesh_arena_pointer;
    Arena_Allocator *persistent_string_arena_pointer;
    Arena_Allocator *level_mesh_arena_pointer;

    Pool_Allocator *string_pool_pointer;   // pool with block size of MAX_ASSET_STRING_SIZE
    Pool_Allocator *filename_pool_pointer; // pool with block size of PLATFORM_MAX_PATH

    Hash_Table<int32, Mesh> mesh_table;
};

int32 add_level_mesh(Asset_Manager *asset_manager, char *filename, char *name) {
    String_Buffer filename_buffer = make_string_buffer((Allocator *) asset_manager->filename_pool_pointer,
                                                       filename, PLATFORM_MAX_PATH);
    String_Buffer mesh_name_buffer = make_string_buffer((Allocator *) asset_manager->string_pool_pointer,
                                                        name, MAX_ASSET_STRING_SIZE);

    Mesh mesh = read_and_load_mesh((Allocator *) asset_manager->level_mesh_arena_pointer,
                                   Mesh_Type::LEVEL,
                                   filename_buffer, mesh_name_buffer);

    Hash_Table<int32, Mesh> *mesh_table = &asset_manager->mesh_table;
    int32 mesh_id = mesh_table->total_added_ever;
    hash_table_add(mesh_table, mesh_id, mesh);

    return mesh_id;
}

int32 add_primitive_mesh(Asset_Manager *asset_manager, char *filename, char *name) {
    Allocator *string_allocator = (Allocator *) asset_manager->persistent_string_arena_pointer;
    String_Buffer filename_buffer = make_string_buffer(string_allocator, filename, PLATFORM_MAX_PATH);
    String_Buffer mesh_name_buffer = make_string_buffer(string_allocator, name, MAX_ASSET_STRING_SIZE);

    Allocator *mesh_allocator = (Allocator *) asset_manager->persistent_mesh_arena_pointer;
    Mesh mesh = read_and_load_mesh(mesh_allocator,
                                   Mesh_Type::PRIMITIVE,
                                   filename_buffer, mesh_name_buffer);

    Hash_Table<int32, Mesh> *mesh_table = &asset_manager->mesh_table;
    int32 mesh_id = mesh_table->total_added_ever;
    hash_table_add(mesh_table, mesh_id, mesh);

    return mesh_id;
}

int32 add_engine_mesh(Asset_Manager *asset_manager, char *filename, char *name) {
    // TODO: in OpenGL code, we can just differentiate between them. it's nicer to just have a single one
    //       since we don't have to constantly switch on type because we have to use different tables.
    Allocator *string_allocator = (Allocator *) asset_manager->persistent_string_arena_pointer;
    String_Buffer filename_buffer = make_string_buffer(string_allocator, filename, PLATFORM_MAX_PATH);
    String_Buffer mesh_name_buffer = make_string_buffer(string_allocator, name, MAX_ASSET_STRING_SIZE);

    Allocator *mesh_allocator = (Allocator *) asset_manager->persistent_mesh_arena_pointer;
    Mesh mesh = read_and_load_mesh(mesh_allocator,
                                   Mesh_Type::ENGINE,
                                   filename_buffer, mesh_name_buffer);

    Hash_Table<int32, Mesh> *mesh_table = &asset_manager->mesh_table;
    int32 mesh_id = mesh_table->total_added_ever;
    hash_table_add(mesh_table, mesh_id, mesh);

    return mesh_id;
}

#endif
