#include "asset.h"

Asset_Manager make_asset_manager(Allocator *allocator) {
    Asset_Manager asset_manager = {};
    asset_manager.allocator_pointer = allocator;
    asset_manager.font_table = make_hash_table<String, Font>((Allocator *) &memory.hash_table_stack,
                                                             HASH_TABLE_SIZE,
                                                             &string_equals);
    asset_manager.font_file_table = make_hash_table<String, File_Data>((Allocator *) &memory.hash_table_stack,
                                                                       HASH_TABLE_SIZE,
                                                                       &string_equals);
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
    font.name = make_string_buffer((Allocator *) &memory.string_arena, font_name, FONT_NAME_MAX_SIZE);
    
    hash_table_add(&asset_manager->font_table, make_string(font.name), font);
    
    return font;
}

Font get_font(Asset_Manager *asset_manager, char *font_name) {
    Font font;
    bool32 font_exists = hash_table_find(asset_manager->font_table, make_string(font_name), &font);
    assert(font_exists);
    return font;
}
