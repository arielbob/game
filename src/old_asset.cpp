#include "asset.h"
#include "level.h"

Material default_material = {
    make_string(""),
    -1,
    0.5f,
    make_vec4(0.5f, 0.5f, 0.5f, 1.0f),
    true
};

Asset_Manager make_asset_manager(Allocator *allocator) {
    Asset_Manager asset_manager = {};
    asset_manager.allocator_pointer = allocator;

    // i think it's probably fine that we use the same allocator to store the hash tables
    asset_manager.mesh_table = make_hash_table<int32, Mesh>(        allocator, HASH_TABLE_SIZE, &int32_equals);
    asset_manager.material_table = make_hash_table<int32, Material>(allocator, HASH_TABLE_SIZE, &int32_equals);
    asset_manager.texture_table = make_hash_table<int32, Texture>(  allocator, HASH_TABLE_SIZE, &int32_equals);

    asset_manager.font_table = make_hash_table<int32, Font>(           allocator, HASH_TABLE_SIZE, &int32_equals);
    asset_manager.font_file_table = make_hash_table<String, File_Data>(allocator, HASH_TABLE_SIZE, &string_equals);

    return asset_manager;
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
        if (string_equals(value->name, name)) {
            return true;
        }
    }

    return false;
}

bool32 generate_mesh_name(Asset_Manager *asset_manager, char *buffer, int32 buffer_size) {
    int32 num_attempts = 0;
    while (num_attempts < MAX_MESHES + 1) {
        char *format = (num_attempts == 0) ? "New Mesh" : "New Mesh %d";
        string_format(buffer, buffer_size, format, num_attempts + 1);
        if (!mesh_name_exists(asset_manager, make_string(buffer))) {
            return true;
        }

        num_attempts++;
    }

    assert(!"Could not generate mesh name.");
    return false;
}

Mesh get_mesh_by_name(Asset_Manager *asset_manager, String mesh_name, int32 *mesh_id) {
    Hash_Table<int32, Mesh> mesh_table = asset_manager->mesh_table;

    FOR_ENTRY_POINTERS(int32, Mesh, mesh_table) {
        if (string_equals(entry->value.name, mesh_name)) {
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
            if (string_equals(entry->value.name, mesh_name)) {
                return entry->key;
            }
        }
    }

    assert(false);
    return -1;
}

inline int32 get_mesh_id_by_name(Asset_Manager *asset_manager, char *mesh_name) {
    return get_mesh_id_by_name(asset_manager, make_string(mesh_name));
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

Font load_font(Asset_Manager *asset_manager,
               char *font_filename, char *font_name,
               real32 font_height_pixels,
               int32 font_texture_width, int32 font_texture_height) {
    Font font = {};
    font.height_pixels = font_height_pixels;
    font.texture_width = font_texture_width;
    font.texture_height = font_texture_height;
    
    stbtt_fontinfo font_info;
    
    File_Data font_file_data;
    String font_filename_string = make_string(font_filename);
    if (!hash_table_find(asset_manager->font_file_table, font_filename_string, &font_file_data)) {
        font_file_data = platform_open_and_read_file((Allocator *) &memory.font_arena,
                                                     font_filename);
        hash_table_add(&asset_manager->font_file_table, font_filename_string, font_file_data);
    }
    
    // get font info
    // NOTE: this assumes that the TTF file only has a single font and is at index 0, or else
    //       stbtt_GetFontOffsetForIndex will return a negative value.
    // NOTE: font_info uses the raw data from the file contents, so the file data allocation should NOT
    //       be temporary.
    stbtt_InitFont(&font_info, (uint8 *) font_file_data.contents,
                   stbtt_GetFontOffsetForIndex((uint8 *) font_file_data.contents, 0));
    font.scale_for_pixel_height = stbtt_ScaleForPixelHeight(&font_info, font_height_pixels);
    stbtt_GetFontVMetrics(&font_info, &font.ascent, &font.descent, &font.line_gap);
    font.font_info = font_info;
    font.file_data = font_file_data;
    
    int32 first_char = 32;
    int32 num_chars = 96;
    font.cdata = (stbtt_bakedchar *) arena_push(&memory.font_arena, num_chars * sizeof(stbtt_bakedchar), false);
    font.first_char = first_char;
    font.num_chars = num_chars;

    // NOTE: these font names are expected to be char array constants
    font.name = make_string(font_name);
    
    // TODO: make sure font with same name doesn't already exist
    int32 font_id = asset_manager->font_table.total_added_ever;
    hash_table_add(&asset_manager->font_table, font_id, font);
    
    return font;
}

Font get_font(Asset_Manager *asset_manager, int32 id) {
    Font font;
    bool32 font_exists = hash_table_find(asset_manager->font_table, id, &font);
    assert(font_exists);
    return font;
}

Font get_font(Asset_Manager *asset_manager, char *font_name, int32 *id = NULL) {
    Hash_Table<int32, Font> font_table = asset_manager->font_table;

    FOR_ENTRY_POINTERS(int32, Font, font_table) {
        if (string_equals(entry->value.name, font_name)) {
            if (id) {
                *id = entry->key;
            }
            return entry->value;
        }
    }

    assert(false);
    return {};
}

int32 add_mesh(Asset_Manager *asset_manager, Mesh mesh, int32 existing_id = -1) {
    int32 id;
    if (existing_id >= 0) {
        id = existing_id;
    } else {
        id = asset_manager->mesh_table.total_added_ever;
    }
    
    hash_table_add(&asset_manager->mesh_table, id, mesh);
    return id;
}

void delete_mesh(Asset_Manager *asset_manager, int32 mesh_id) {
    hash_table_remove(&asset_manager->mesh_table, mesh_id);
}

int32 add_texture(Asset_Manager *asset_manager, Texture texture, int32 existing_id = -1) {
    int32 id;
    if (existing_id >= 0) {
        id = existing_id;
    } else {
        id = asset_manager->texture_table.total_added_ever;
    }

    hash_table_add(&asset_manager->texture_table, id, texture);
    return id;
}

void delete_texture(Asset_Manager *asset_manager, int32 texture_id) {
    hash_table_remove(&asset_manager->texture_table, texture_id);
}

Texture get_texture(Asset_Manager *asset_manager, int32 texture_id) {
    Texture texture;
    bool32 texture_exists = hash_table_find(asset_manager->texture_table,
                                            texture_id,
                                            &texture);
    assert(texture_exists);
    return texture;
}

Texture *get_texture_pointer(Asset_Manager *asset_manager, int32 texture_id) {
    Texture *texture;
    bool32 texture_exists = hash_table_find_pointer(asset_manager->texture_table,
                                                     texture_id,
                                                     &texture);
    assert(texture_exists);
    return texture;
}

int32 get_texture_id_by_name(Asset_Manager *asset_manager, String texture_name) {
    Hash_Table<int32, Texture> texture_table = asset_manager->texture_table;

    FOR_ENTRY_POINTERS(int32, Texture, texture_table) {
        if (string_equals(entry->value.name, texture_name)) {
            return entry->key;
        }
    }

    assert(false);
    return -1;
}

int32 texture_name_exists(Asset_Manager *asset_manager, String texture_name) {
    Hash_Table<int32, Texture> texture_table = asset_manager->texture_table;

    FOR_ENTRY_POINTERS(int32, Texture, texture_table) {
        if (string_equals(entry->value.name, texture_name)) {
            return true;
        }
    }

    return false;
}

int32 add_material(Asset_Manager *asset_manager, Material material, int32 existing_id = -1) {
    int32 id;
    if (existing_id >= 0) {
        id = existing_id;
    } else {
        id = asset_manager->material_table.total_added_ever;
    }

    hash_table_add(&asset_manager->material_table, id, material);
    return id;
}

void delete_material(Asset_Manager *asset_manager, int32 material_id) {
    hash_table_remove(&asset_manager->material_table, material_id);
}

Material get_material(Asset_Manager *asset_manager, int32 material_id) {
    Material material;
    bool32 material_exists = hash_table_find(asset_manager->material_table,
                                             material_id,
                                             &material);
    assert(material_exists);
    return material;
}

Material *get_material_pointer(Asset_Manager *asset_manager, int32 material_id) {
    Material *material;
    bool32 material_exists = hash_table_find_pointer(asset_manager->material_table,
                                                     material_id,
                                                     &material);
    assert(material_exists);
    return material;
}

int32 get_material_id_by_name(Asset_Manager *asset_manager, String material_name) {
    Hash_Table<int32, Material> material_table = asset_manager->material_table;

    FOR_ENTRY_POINTERS(int32, Material, material_table) {
        if (string_equals(entry->value.name, material_name)) {
            return entry->key;
        }
    }

    assert(false);
    return -1;
}

int32 material_name_exists(Asset_Manager *asset_manager, String material_name) {
    Hash_Table<int32, Material> material_table = asset_manager->material_table;

    FOR_ENTRY_POINTERS(int32, Material, material_table) {
        if (string_equals(entry->value.name, material_name)) {
            return true;
        }
    }

    return false;
}

bool32 generate_material_name(Asset_Manager *asset_manager, char *buffer, int32 buffer_size) {
    int32 num_attempts = 0;
    while (num_attempts < MAX_MATERIALS + 1) {
        char *format = (num_attempts == 0) ? "New Material" : "New Material %d";
        string_format(buffer, buffer_size, format, num_attempts + 1);
        if (!material_name_exists(asset_manager, make_string(buffer))) {
            return true;
        }

        num_attempts++;
    }

    assert(!"Could not generate material name.");
    return false;
}

bool32 generate_texture_name(Asset_Manager *asset_manager, char *buffer, int32 buffer_size) {
    int32 num_attempts = 0;
    while (num_attempts < MAX_TEXTURES + 1) {
        char *format = (num_attempts == 0) ? "New Texture" : "New Texture %d";
        string_format(buffer, buffer_size, format, num_attempts + 1);
        if (!texture_name_exists(asset_manager, make_string(buffer))) {
            return true;
        }

        num_attempts++;
    }

    assert(!"Could not generate texture name.");
    return false;
}

void set_texture(Material *material, int32 texture_id, int32 *original_texture_id = NULL) {
    if (original_texture_id) {
        *original_texture_id = material->texture_id;
    }
    material->texture_id = texture_id;
}

Mesh read_and_load_mesh(Allocator *allocator, String filename, String name, Mesh_Type type) {
    Marker m = begin_region();

    char *c_filename = to_char_array(temp_region, filename);
    File_Data file_data = platform_open_and_read_file(temp_region, c_filename);

    Mesh mesh = Mesh_Loader::load_mesh(file_data, allocator);
    mesh.allocator = allocator;
    mesh.filename = filename;
    mesh.name = name;
    mesh.type = type;

    end_region(m);

    return mesh;
}

inline Mesh read_and_load_mesh(Allocator *allocator, char *c_filename, char *c_name, Mesh_Type type) {
    String filename = copy(allocator, make_string(c_filename));
    String name = copy(allocator, make_string(c_name));
    return read_and_load_mesh(allocator, filename, name, type);
}

void load_default_assets(Asset_Manager *asset_manager) {
    Allocator *allocator = asset_manager->allocator_pointer;

    Mesh mesh = read_and_load_mesh(allocator, "blender/gizmo_arrow.mesh", "gizmo_arrow", Mesh_Type::ENGINE);
    add_mesh(asset_manager, mesh);

    mesh = read_and_load_mesh(allocator, "blender/gizmo_ring.mesh", "gizmo_ring", Mesh_Type::ENGINE);
    add_mesh(asset_manager, mesh);

    mesh = read_and_load_mesh(allocator, "blender/gizmo_sphere.mesh", "gizmo_sphere", Mesh_Type::ENGINE);
    add_mesh(asset_manager, mesh);

    mesh = read_and_load_mesh(allocator, "blender/gizmo_cube.mesh", "gizmo_cube", Mesh_Type::ENGINE);
    add_mesh(asset_manager, mesh);

    mesh = read_and_load_mesh(allocator, "blender/cube.mesh", "cube", Mesh_Type::PRIMITIVE);
    add_mesh(asset_manager, mesh);

    // default fonts
    load_font(asset_manager, "c:/windows/fonts/times.ttf", "times32", 32.0f, 512, 512);
    load_font(asset_manager, "c:/windows/fonts/times.ttf", "times24", 24.0f, 512, 512);

    load_font(asset_manager, "c:/windows/fonts/courbd.ttf", "courier24b", 24.0f, 512, 512);

    load_font(asset_manager, "c:/windows/fonts/calibri.ttf", "calibri14", 14.0f, 512, 512);
    load_font(asset_manager, "c:/windows/fonts/calibrib.ttf", "calibri14b", 14.0f, 512, 512);
    load_font(asset_manager, "c:/windows/fonts/calibrib.ttf", "calibri24b", 24.0f, 512, 512);

    load_font(asset_manager, "c:/windows/fonts/lucon.ttf", "lucidaconsole18", 18.0f, 512, 512);
}

void unload_level_assets(Asset_Manager *asset_manager) {
    // we deallocate the assets
    // then in opengl code, we just unload all the level assets from the GPU, we don't need
    // the asset data at that point since we know which resources are for levels and which are not
    // TODO: delete level meshes, materials, textures, with new format
    
    Hash_Table<int32, Mesh> *mesh_table = &asset_manager->mesh_table;
    int32 num_reset = 0;
    FOR_ENTRY_POINTERS(int32, Mesh, *mesh_table) {
        if (entry->value.type != Mesh_Type::LEVEL) continue;

        Mesh mesh = entry->value;
        deallocate(mesh);
        entry->is_occupied = false;
        num_reset++;
    }
    mesh_table->num_entries -= num_reset;
    assert(mesh_table->num_entries >= 0);
    
    reset_and_deallocate_entries(&asset_manager->texture_table);
    reset_and_deallocate_entries(&asset_manager->material_table);
}

bool32 load_level_assets(Asset_Manager *asset_manager, Level_Info *level_info) {
    FOR_LIST_NODES(Mesh_Info, level_info->meshes) {
        Allocator *allocator = asset_manager->allocator_pointer;

        Mesh_Info mesh_info = current_node->value;

        String filename = copy(allocator, mesh_info.filename);
        String name = copy(allocator, mesh_info.name);

        Mesh mesh = read_and_load_mesh(allocator, filename, name, Mesh_Type::LEVEL);
        add_mesh(asset_manager, mesh);
    }

    FOR_LIST_NODES(Texture_Info, level_info->textures) {
        Texture_Info texture_info = current_node->value;
        
        Allocator *allocator = asset_manager->allocator_pointer;
        String name = copy(allocator, texture_info.name);
        String filename = copy(allocator, texture_info.filename);
        Texture texture = make_texture(name, filename);

        add_texture(asset_manager, texture);
    }

    FOR_LIST_NODES(Material_Info, level_info->materials) {
        Material_Info material_info = current_node->value;

        int32 texture_id = -1;
        if (material_info.flags & HAS_TEXTURE) {
            texture_id = get_texture_id_by_name(asset_manager, material_info.texture_name);
        }
        
        Allocator *allocator = asset_manager->allocator_pointer;
        Material material = material_info.material;
        material.texture_id = texture_id;
        material.name = copy(allocator, material_info.name);

        add_material(asset_manager, material);
    }

    return true;
}
