#include "asset.h"

Mesh *get_mesh(String name) {
    uint32 hash = get_hash(name, NUM_MESH_BUCKETS);

    Mesh *current = asset_manager->mesh_table[hash];
    while (current) {
        if (string_equals(current->name, name)) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

void delete_mesh(String name) {
    uint32 hash = get_hash(name, NUM_MESH_BUCKETS);

    Mesh *current = asset_manager->mesh_table[hash];
    while (current) {
        if (string_equals(current->name, name)) {
            if (current->table_prev) {
                current->table_prev->table_next = current->table_next;
            }
            if (current->table_next) {
                current->table_next->table_prev = current->table_prev;
            }

            deallocate(current->name);
            deallocate(asset_manager->allocator, current->data);
            deallocate(asset_manager->allocator, current->indices);
            deallocate(asset_manager->allocator, current);
            return;
        }

        current = current->table_next;
    }

    assert(!"Mesh does not exist.");
}

Mesh load_mesh(Allocator *allocator, Mesh_Type type, String name, String filename) {
    Marker m = begin_region();

    char *c_filename = to_char_array(temp_region, filename);
    File_Data file_data = platform_open_and_read_file(temp_region, filename);

    Mesh mesh = Mesh_Loader::load_mesh(file_data, allocator);
    mesh.name = name;
    mesh.type = type;

    end_region(m);

    return mesh;
}

bool32 mesh_exists(String name) {
    uint32 hash = get_hash(name, NUM_FONT_BUCKETS);

    Mesh *current = asset_manager->mesh_table[hash];
    while (current) {
        if (string_equals(current->name, name)) {
            return true;
        }

        current = current->table_next;
    }

    return false;
}

Mesh *add_mesh(Mesh_Type type, String name, String filename) {
    Mesh *mesh = (Mesh *) allocate(asset_manager->allocator, sizeof(Mesh));
    *mesh = load_mesh(asset_manager->allocator, type, name, filename);

    if (mesh_exists(name)) {
        assert(!"Mesh with name already exists.");
        return NULL;
    }
    
    uint32 hash = get_hash(name, NUM_MESH_BUCKETS);

    Mesh *current = asset_manager->mesh_table[hash];
    mesh->table_next = current;
    mesh->table_prev = NULL;
    if (current) {
        current->table_prev = mesh;
    }
    asset_manager->mesh_table[hash] = mesh;

    return mesh;
}

Font_File *get_font_file(char *filename) {
    uint32 hash = get_hash(make_string(filename), NUM_FONT_FILE_BUCKETS);

    Font_File *current = asset_manager->font_file_table[hash];
    while (current) {
        if (string_equals(current->filename, filename)) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

Font_File *add_font_file(char *filename, File_Data font_file_data) {
    if (get_font_file(filename)) {
        assert(!"Font file already exists.");
        return NULL;
    }

    Font_File *font_file = (Font_File *) allocate(asset_manager->allocator, sizeof(Font_File));
    uint32 hash = get_hash(filename, NUM_FONT_FILE_BUCKETS);

    Font_File *current = asset_manager->font_file_table[hash];
    font_file->table_next = current;
    font_file->table_prev = NULL;
    font_file->filename = filename;
    font_file->file_data = font_file_data;
    if (current) {
        current->table_prev = font_file;
    }

    asset_manager->font_file_table[hash] = font_file;

    return font_file;
}

bool32 font_exists(char *font_name) {
    uint32 hash = get_hash(font_name, NUM_FONT_BUCKETS);

    Font *current = asset_manager->font_table[hash];
    while (current) {
        if (string_equals(current->name, font_name)) {
            return true;
        }

        current = current->table_next;
    }

    return false;
}

Font *add_font(char *font_name, char *font_filename,
              real32 font_height_pixels,
              int32 font_texture_width = 512, int32 font_texture_height = 512) {
    if (font_exists(font_name)) {
        assert(!"Font with name already exists.");
        return NULL;
    }

    Font *font = (Font *) allocate(asset_manager->allocator, sizeof(Font), true);
    
    font->height_pixels = font_height_pixels;
    font->texture_width = font_texture_width;
    font->texture_height = font_texture_height;
    
    File_Data font_file_data;
    Font_File *font_file = get_font_file(font_filename);
    if (!font_file) {
        font_file_data = platform_open_and_read_file(asset_manager->allocator, font_filename);
        add_font_file(font_filename, font_file_data);
    } else {
        font_file_data = font_file->file_data;
    }

    // get font info
    stbtt_fontinfo font_info;

    // NOTE: this assumes that the TTF file only has a single font and is at index 0, or else
    //       stbtt_GetFontOffsetForIndex will return a negative value.
    // NOTE: font_info uses the raw data from the file contents, so the file data allocation should NOT
    //       be temporary.
    stbtt_InitFont(&font_info, (uint8 *) font_file_data.contents,
                   stbtt_GetFontOffsetForIndex((uint8 *) font_file_data.contents, 0));
    font->scale_for_pixel_height = stbtt_ScaleForPixelHeight(&font_info, font_height_pixels);
    stbtt_GetFontVMetrics(&font_info, &font->ascent, &font->descent, &font->line_gap);
    font->font_info = font_info;
    font->file_data = font_file_data;

    // allocate memory for the baked characters, but we bake in opengl code
    int32 first_char = 32;
    int32 num_chars = 96;
    font->cdata = (stbtt_bakedchar *) allocate(asset_manager->allocator, num_chars * sizeof(stbtt_bakedchar), false);
    font->first_char = first_char;
    font->num_chars = num_chars;

    // NOTE: these font names are expected to be char array constants
    font->name = make_string(font_name);

    // add to table
    uint32 hash = get_hash(font_name, NUM_FONT_BUCKETS);
    Font *current = asset_manager->font_table[hash];
    font->table_next = current;
    font->table_prev = NULL;
    
    if (current) {
        current->table_prev = font;
    }
    asset_manager->font_table[hash] = font;
    
    return font;
}

Font *get_font(char *name) {
    uint32 hash = get_hash(name, NUM_FONT_BUCKETS);

    Font *current = asset_manager->font_table[hash];
    while (current) {
        if (string_equals(current->name, name)) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}
