#ifndef ASSET_H
#define ASSET_H

#include "mesh.h"
#include "font.h"

#define MAX_MESHES 64
#define MAX_ASSET_STRING_SIZE 64

struct Texture {
    String name;
    String filename;
    bool32 is_loaded;
    bool32 should_unload;
};

Texture make_texture(String name, String filename) {
    Texture result = {};
    result.name = name;
    result.filename = filename;
    return result;
}

Texture copy(Allocator *allocator, Texture texture) {
    Texture result = texture;
    result.name = copy(allocator, texture.name);
    result.filename = copy(allocator, texture.filename);
    return result;
}

void deallocate(Texture texture) {
    deallocate(texture.name);
    deallocate(texture.filename);
}

struct Material {
    String name;
    int32 texture_id;
    real32 gloss;
    Vec4 color_override;
    bool32 use_color_override;
};

Material make_material() {
    Material result = {};
    result.texture_id = -1;
    return result;
}

Material make_material(String name, int32 texture_id, real32 gloss, Vec4 color_override, bool32 use_color_override) {
    Material result = {
        name, texture_id, gloss, color_override, use_color_override
    };
    return result;
}

Material copy(Allocator *allocator, Material material) {
    Material result = material;
    result.name = copy(allocator, material.name);
    return result;
}

void deallocate(Material material) {
    deallocate(material.name);
}

struct Asset_Manager {
    Allocator *allocator_pointer;
    
    Hash_Table<int32, Mesh> mesh_table;
    Hash_Table<int32, Material> material_table;
    Hash_Table<int32, Texture> texture_table;

    Hash_Table<String, File_Data> font_file_table;
    Hash_Table<int32, Font> font_table;
};

#endif
