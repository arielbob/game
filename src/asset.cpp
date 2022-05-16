#include "asset.h"

// NOTE: filenames should always be relative, since that's what we put into the level file when saving levels.
Mesh add_level_mesh(Asset_Manager *asset_manager, String filename, String name, int32 *result_id = NULL) {
    String_Buffer filename_buffer = make_string_buffer((Allocator *) asset_manager->filename_pool_pointer,
                                                       filename, PLATFORM_MAX_PATH);
    String_Buffer mesh_name_buffer = make_string_buffer((Allocator *) asset_manager->string_pool_pointer,
                                                        name, MAX_ASSET_STRING_SIZE);

    Mesh mesh = read_and_load_mesh((Allocator *) asset_manager->level_mesh_heap_pointer,
                                   Mesh_Type::LEVEL,
                                   filename_buffer, mesh_name_buffer);

    Hash_Table<int32, Mesh> *mesh_table = &asset_manager->mesh_table;
    int32 mesh_id = mesh_table->total_added_ever;
    hash_table_add(mesh_table, mesh_id, mesh);

    if (result_id) {
        *result_id = mesh_id;
    }

    return mesh;
}

inline Mesh add_level_mesh(Asset_Manager *asset_manager, char *filename, char *name, int32 *result_id = NULL) {
    return add_level_mesh(asset_manager, make_string(filename), make_string(name), result_id);
}

Mesh add_primitive_mesh(Asset_Manager *asset_manager, char *filename, char *name, int32 *result_id = NULL) {
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

    if (result_id) {
        *result_id = mesh_id;
    }

    return mesh;
}

Mesh add_engine_mesh(Asset_Manager *asset_manager, char *filename, char *name, int32 *result_id = NULL) {
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

    if (result_id) {
        *result_id = mesh_id;
    }

    return mesh;
}

// NOTE: doesn't delete, since we clear level meshes all at once since they're all stored with the same allocator
void reset_mesh_table_level_entries(Asset_Manager *asset_manager) {
    int32 num_reset = 0;
    FOR_ENTRY_POINTERS(int32, Mesh, asset_manager->mesh_table) {
        if (entry->value.type == Mesh_Type::LEVEL) {
            entry->is_occupied = false;
            num_reset++;
        }
    }

    asset_manager->mesh_table.num_entries -= num_reset;
    assert(asset_manager->mesh_table.num_entries >= 0);
}

bool32 mesh_name_exists(Asset_Manager *asset_manager, String name) {
    FOR_VALUE_POINTERS(int32, Mesh, asset_manager->mesh_table) {
        if (string_equals(make_string(value->name), name)) {
            return true;
        }
    }

    return false;
}

Mesh get_mesh_by_name(Asset_Manager *asset_manager, String mesh_name, int32 *mesh_id) {
    Hash_Table<int32, Mesh> mesh_table = asset_manager->mesh_table;

    FOR_ENTRY_POINTERS(int32, Mesh, mesh_table) {
        if (string_equals(make_string(entry->value.name), mesh_name)) {
            *mesh_id = entry->key;
            return entry->value;
        }
    }

    assert(false);
    return {};
}

int32 get_mesh_id_by_name(Asset_Manager *asset_manager, String mesh_name) {
    Hash_Table<int32, Mesh> mesh_table = asset_manager->mesh_table;

    FOR_ENTRY_POINTERS(int32, Mesh, mesh_table) {
        if (entry->is_occupied) {
            if (string_equals(make_string(entry->value.name), mesh_name)) {
                return entry->key;
            }
        }
    }

    assert(false);
    return -1;
}

bool32 mesh_exists(Asset_Manager *asset_manager, int32 mesh_id) {
    return hash_table_exists(asset_manager->mesh_table, mesh_id);
}

Mesh get_mesh(Asset_Manager *asset_manager, int32 mesh_id) {
    Mesh mesh;
    bool32 mesh_exists = false;
    mesh_exists = hash_table_find(asset_manager->mesh_table, mesh_id, &mesh);
    assert(mesh_exists);
    return mesh;
}

Mesh *get_mesh_pointer(Asset_Manager *asset_manager, int32 mesh_id) {
    Mesh *mesh = NULL;
    bool32 mesh_exists = false;
    mesh_exists = hash_table_find_pointer(asset_manager->mesh_table, mesh_id, &mesh);
    assert(mesh_exists);
    return mesh;
}
