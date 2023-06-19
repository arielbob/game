#include "asset.h"
#include "entity.h"

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
            } else {
                // if we're first in list, we need to update bucket array when we delete
                asset_manager->mesh_table[hash] = current->table_next;
            }
            
            if (current->table_next) {
                current->table_next->table_prev = current->table_prev;
            }

            deallocate(current);
            deallocate(asset_manager->allocator, current);

            r_unload_mesh(name);
            
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
    mesh.type     = type;
    mesh.name     = copy(allocator, name);
    mesh.filename = copy(allocator, filename);

    end_region(m);

    return mesh;
}

bool32 mesh_exists(String name) {
    uint32 hash = get_hash(name, NUM_MESH_BUCKETS);

    Mesh *current = asset_manager->mesh_table[hash];
    while (current) {
        if (string_equals(current->name, name)) {
            return true;
        }

        current = current->table_next;
    }

    return false;
}

Mesh *add_mesh(String name, String filename, Mesh_Type type) {
    if (mesh_exists(name)) {
        assert(!"Mesh with name already exists.");
        return NULL;
    }

    Mesh *mesh = (Mesh *) allocate(asset_manager->allocator, sizeof(Mesh));
    *mesh = load_mesh(asset_manager->allocator, type, name, filename);
    
    uint32 hash = get_hash(name, NUM_MESH_BUCKETS);

    Mesh *current = asset_manager->mesh_table[hash];
    mesh->table_next = current;
    mesh->table_prev = NULL;
    if (current) {
        current->table_prev = mesh;
    }
    asset_manager->mesh_table[hash] = mesh;

    r_load_mesh(mesh->name);
    
    return mesh;
}

inline Mesh *add_mesh(char *name, char *filename, Mesh_Type type) {
    return add_mesh(make_string(name), make_string(filename), type);
}

Texture *get_texture(String name) {
    uint32 hash = get_hash(name, NUM_TEXTURE_BUCKETS);

    Texture *current = asset_manager->texture_table[hash];
    while (current) {
        if (string_equals(current->name, name)) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

void get_texture_names(Allocator *allocator, char **names, int max_names, int *num_names) {
    int32 num_texture_names = 0;
    for (int32 i = 0; i < NUM_TEXTURE_BUCKETS; i++) {
        Texture *current = asset_manager->texture_table[i];
        bool32 should_exit = false;
        while (current) {
            if (num_texture_names >= max_names) {
                assert(num_texture_names < max_names);
                should_exit = true;
                break;
            }
            names[num_texture_names++] = to_char_array(allocator, current->name);
            current = current->table_next;
        }

        if (should_exit) {
            break;
        }
    }

    *num_names = num_texture_names;
}

void delete_texture(String name) {
    uint32 hash = get_hash(name, NUM_TEXTURE_BUCKETS);

    Texture *current = asset_manager->texture_table[hash];
    while (current) {
        if (string_equals(current->name, name)) {
            if (current->table_prev) {
                current->table_prev->table_next = current->table_next;
            } else {
                // if we're first in list, we need to update bucket array when we delete
                asset_manager->texture_table[hash] = current->table_next;
            }
            
            if (current->table_next) {
                current->table_next->table_prev = current->table_prev;
            }

            deallocate(current->name);
            deallocate(current->filename);
            deallocate(asset_manager->allocator, current);

            r_unload_texture(name);
            
            return;
        }

        current = current->table_next;
    }

    assert(!"Texture does not exist.");
}

bool32 texture_exists(String name) {
    uint32 hash = get_hash(name, NUM_TEXTURE_BUCKETS);

    Texture *current = asset_manager->texture_table[hash];
    while (current) {
        if (string_equals(current->name, name)) {
            return true;
        }

        current = current->table_next;
    }

    return false;
}

Texture *add_texture(String name, String filename, Texture_Type type) {
    if (texture_exists(name)) {
        assert(!"Texture with name already exists.");
        return NULL;
    }

    Texture *texture  = (Texture *) allocate(asset_manager->allocator, sizeof(Texture), true);
    texture->name     = copy(asset_manager->allocator, name);
    texture->filename = copy(asset_manager->allocator, filename);
    texture->type     = type;
    
    uint32 hash = get_hash(name, NUM_TEXTURE_BUCKETS);

    Texture *current = asset_manager->texture_table[hash];
    texture->table_next = current;
    texture->table_prev = NULL;
    if (current) {
        current->table_prev = texture;
    }
    asset_manager->texture_table[hash] = texture;

    r_load_texture(name);
    
    return texture;
}

inline Texture *add_texture(char *name, char *filename, Texture_Type type) {
    return add_texture(make_string(name), make_string(filename), type);
}

bool32 material_exists(String name) {
    for (int32 i = 0; i < NUM_MATERIAL_BUCKETS; i++) {
        Material *current = asset_manager->material_table[i];
        while (current) {
            if (string_equals(current->name, name)) {
                return true;
            }
            current = current->table_next;
        }
    }

    return false;
}

Material *add_material(Material_Info *material_info, Material_Type type) {
    if (material_exists(material_info->name)) {
        assert(!"Material with name already exists.");
        return NULL;
    }

    Allocator *allocator = asset_manager->allocator;
    Material *material = (Material *) allocate(allocator, sizeof(Material), true);

    material->type                   = type;
    material->id                     = asset_manager->total_materials_added_ever++;
    material->name                   = copy(allocator, material_info->name);
    material->flags                  = material_info->flags;
        
    material->albedo_texture_name    = copy(allocator, material_info->albedo_texture_name);
    material->albedo_color           = material_info->albedo_color;
        
    material->metalness_texture_name = copy(allocator, material_info->metalness_texture_name);
    material->metalness              = material_info->metalness;

    material->roughness_texture_name = copy(allocator, material_info->roughness_texture_name);
    material->roughness              = material_info->roughness;

    uint32 hash = get_hash(material->id, NUM_MATERIAL_BUCKETS);

    Material *current = asset_manager->material_table[hash];
    material->table_next = current;
    material->table_prev = NULL;
    if (current) {
        current->table_prev = material;
    }
    asset_manager->material_table[hash] = material;

    return material;
}

Material *get_material(String name) {
    for (int32 i = 0; i < NUM_MATERIAL_BUCKETS; i++) {
        Material *current = asset_manager->material_table[i];
        while (current) {
            if (string_equals(current->name, name)) {
                return current;
            }
            current = current->table_next;
        }
    }

    return NULL;
}

Material *get_material(int32 id) {
    uint32 hash = get_hash(id, NUM_MATERIAL_BUCKETS);

    Material *current = asset_manager->material_table[hash];
    while (current) {
        if (current->id == id) {
            return current;
        }

        current = current->table_next;
    }

    return NULL;
}

void get_material_names(Allocator *allocator, char **names, int max_names, int *num_names) {
    int32 num_material_names = 0;
    for (int32 i = 0; i < NUM_MATERIAL_BUCKETS; i++) {
        Material *current = asset_manager->material_table[i];
        bool32 should_exit = false;
        while (current) {
            if (num_material_names >= max_names) {
                assert(num_material_names < max_names);
                should_exit = true;
                break;
            }
            names[num_material_names++] = to_char_array(allocator, current->name);
            current = current->table_next;
        }

        if (should_exit) {
            break;
        }
    }

    *num_names = num_material_names;
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

    // allocate memory for the baked characters
    int32 first_char = 32;
    int32 num_chars = 96;
    font->cdata = (stbtt_bakedchar *) allocate(asset_manager->allocator, num_chars * sizeof(stbtt_bakedchar), false);
    font->first_char = first_char;
    font->num_chars = num_chars;

    // NOTE: these font names are expected to be char array constants
    font->name = make_string(font_name);

    // bake it
    font->bitmap = (uint8 *) allocate(asset_manager->allocator, font->texture_width*font->texture_height);
    // NOTE: no guarantee that the bitmap will fit the font, so choose temp_bitmap dimensions carefully
    // TODO: we may want to maybe render this out to an image so that we can verify that the font fits
    int32 result = stbtt_BakeFontBitmap((uint8 *) font->file_data.contents, 0,
                                        font->height_pixels,
                                        font->bitmap, font->texture_width, font->texture_height,
                                        font->first_char, font->num_chars,
                                        font->cdata);
    assert(result > 0);
    font->is_baked = true;
    
    // add to table
    uint32 hash = get_hash(font_name, NUM_FONT_BUCKETS);
    Font *current = asset_manager->font_table[hash];
    font->table_next = current;
    font->table_prev = NULL;
    
    if (current) {
        current->table_prev = font;
    }
    asset_manager->font_table[hash] = font;

    r_load_font(font->name);
    
    return font;
}

Font *get_font(String name) {
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

void unload_level_meshes() {
    Mesh **mesh_table = asset_manager->mesh_table;
    
    for (int32 i = 0; i < NUM_MESH_BUCKETS; i++) {
        Mesh *current = mesh_table[i];
        while (current) {
            Mesh *next = current->table_next;

            if (current->type == Mesh_Type::LEVEL) {
                if (current->table_prev) {
                    current->table_prev->table_next = next;
                } else {
                    // if we're first in list, we need to update bucket array when we delete
                    mesh_table[i] = current->table_next;
                }
                
                if (current->table_next) {
                    current->table_next->table_prev = current->table_prev;
                }

                r_unload_mesh(current->name);
                deallocate(current);
                deallocate(asset_manager->allocator, current);

                if (current == mesh_table[i]) {
                    // if it's first in the list, then we need to update mesh table when we delete it
                    mesh_table[i] = next;
                }
            }
            
            current = next;
        }
    }
}

void unload_level_materials() {
    Material **material_table = asset_manager->material_table;

    for (int32 i = 0; i < NUM_MATERIAL_BUCKETS; i++) {
        Material *current = material_table[i];
        while (current) {
            Material *next = current->table_next;

            if (current->type == Material_Type::LEVEL) {
                if (current->table_prev) {
                    current->table_prev->table_next = next;
                } else {
                    material_table[i] = current->table_next;
                }
            
                if (current->table_next) {
                    current->table_next->table_prev = current->table_prev;
                }
            
                deallocate(current);
                deallocate(asset_manager->allocator, current);

                if (current == material_table[i]) {
                    // if it's first in the list, then we need to update mesh table when we delete it
                    material_table[i] = next;
                }
            }

            current = next;
        }
    }
}

void unload_level_textures() {
    Texture **texture_table = asset_manager->texture_table;
    
    for (int32 i = 0; i < NUM_TEXTURE_BUCKETS; i++) {
        Texture *current = texture_table[i];
        while (current) {
            Texture *next = current->table_next;

            if (current->type == Texture_Type::LEVEL) {
                if (current->table_prev) {
                    current->table_prev->table_next = next;
                } else {
                    // if we're first in list, we need to update bucket array when we delete
                    texture_table[i] = current->table_next;
                }
                
                if (current->table_next) {
                    current->table_next->table_prev = current->table_prev;
                }

                r_unload_texture(current->name);
                deallocate(current);
                deallocate(asset_manager->allocator, current);

                if (current == texture_table[i]) {
                    // if it's first in the list, then we need to update texture table when we delete it
                    texture_table[i] = next;
                }
            }
            
            current = next;
        }
    }
}

void load_level_assets(Level_Info *level_info) {
    // we always copy strings in here; i don't think functions that take strings should copy them.
    // i think it's better to assume that functions never copy strings and the caller of those
    // functions is responsible to copy them.
    for (int32 i = 0; i < level_info->num_meshes; i++) {
        Mesh_Info *mesh_info = &level_info->meshes[i];
        add_mesh(mesh_info->name,
                 mesh_info->filename,
                 Mesh_Type::LEVEL);
    }

    for (int32 i = 0; i < level_info->num_textures; i++) {
        Texture_Info *texture_info = &level_info->textures[i];
        add_texture(texture_info->name,
                    texture_info->filename,
                    Texture_Type::LEVEL);
    }

    for (int32 i = 0; i < level_info->num_materials; i++) {
        Material_Info *material_info = &level_info->materials[i];
        add_material(material_info, Material_Type::LEVEL);
    }
}

// maybe a better word is delete, since we're just deleting the level assets from their tables
// but then we also unload them on the GPU
void unload_level_assets() {
    // we deallocate the assets
    // then in opengl code, we just unload all the level assets from the GPU, we don't need
    // the asset data at that point since we know which resources are for levels and which are not

    unload_level_meshes();
    unload_level_materials();
    unload_level_textures();
    
    asset_manager->gpu_should_unload_level_assets = true;
}

void load_default_assets() {
    // we need default assets for assets that are based on some file, for example meshes and textures.
    // this is because if we create a new mesh or a new texture, we need some default asset to use/display.
    // i guess we could just make a default texture just be no texture, but that's not ideal, since we would
    // ideally want some type of conspicuous texture that makes it obvious that an entity does not have a
    // texture.
    //
    // we don't need default materials. materials reference meshes and textures but materials themselves
    // don't directly require any files.

    add_mesh("gizmo_arrow",      "blender/gizmo_arrow.mesh",      Mesh_Type::ENGINE);
    add_mesh("gizmo_ring",       "blender/gizmo_ring.mesh",       Mesh_Type::ENGINE);
    add_mesh("gizmo_sphere",     "blender/gizmo_sphere.mesh",     Mesh_Type::ENGINE);
    add_mesh("gizmo_cube",       "blender/gizmo_cube.mesh",       Mesh_Type::ENGINE);
    add_mesh("capsule_cylinder", "blender/capsule_cylinder.mesh", Mesh_Type::ENGINE);
    add_mesh("capsule_cap",      "blender/capsule_cap.mesh",      Mesh_Type::ENGINE);
    add_mesh("cube",             "blender/cube.mesh",             Mesh_Type::PRIMITIVE);
    
    add_texture("texture_default",   "blender/debug-texture.jpg",          Texture_Type::DEFAULT);
    add_texture("lightbulb",         "src/textures/lightbulb.png",         Texture_Type::ENGINE);
    add_texture("editor_down_arrow", "src/textures/editor_down_arrow.png", Texture_Type::ENGINE);
    add_texture("editor_check",      "src/textures/editor_check.png",      Texture_Type::ENGINE);

    add_font("times32",         "c:/windows/fonts/times.ttf",    32.0f);
    add_font("times24",         "c:/windows/fonts/times.ttf",    24.0f);
    add_font("courier24b",      "c:/windows/fonts/courbd.ttf",   24.0f);
    add_font("calibri14",       "c:/windows/fonts/calibri.ttf",  14.0f);
    add_font("calibri14b",      "c:/windows/fonts/calibrib.ttf", 14.0f);
    add_font("calibri24b",      "c:/windows/fonts/calibrib.ttf", 24.0f);
    add_font("lucidaconsole18", "c:/windows/fonts/lucon.ttf",    18.0f);

    Material_Info default_material_info = {};
    
    default_material_info.name = make_string("default_material");
    default_material_info.flags = 0;

    default_material_info.albedo_color = make_vec3(0.5f, 0.5f, 0.5f);
    default_material_info.albedo_texture_name = make_string("texture_default");

    default_material_info.metalness = 0.0f;
    default_material_info.metalness_texture_name = make_string("texture_default");

    default_material_info.roughness = 0.5f;
    default_material_info.roughness_texture_name = make_string("texture_default");

    add_material(&default_material_info, Material_Type::DEFAULT);
}

bool32 generate_asset_name(Allocator *allocator, char *asset_type, int32 max_attempts, String *result) {
    int32 num_attempts = 0;
    Marker m = begin_region();

    String_Buffer buffer = make_string_buffer(temp_region, 256);
    bool success = false;

    char *zero_format = string_format(temp_region, "New %s", asset_type);
    char *n_format = string_format(temp_region, "New %s %%d", asset_type);
    
    while (num_attempts < max_attempts) {
        char *format = (num_attempts == 0) ? zero_format : n_format;
        string_format(&buffer, format, num_attempts + 1);
        if (!material_exists(make_string(buffer))) {
            success = true;
            break;
        }

        num_attempts++;
    }

    if (success) {
        *result = make_string(allocator, buffer);
    } else {
        assert(!"Could not generate material name.");
    }

    end_region(m);

    return success;
}
