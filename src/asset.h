#ifndef ASSET_H
#define ASSET_H

#include "mesh.h"
#include "font.h"

#define MAX_MESHES 64
#define MAX_ASSET_STRING_SIZE 64

#define MATERIAL_USE_ALBEDO_TEXTURE    (1 << 0)
#define MATERIAL_USE_METALNESS_TEXTURE (1 << 1)
#define MATERIAL_USE_ROUGHNESS_TEXTURE (1 << 2)

struct Texture {
    String filename;
    bool32 is_loaded;
    bool32 should_unload;
};

Texture make_texture(String filename) {
    Texture result = {};
    result.filename = filename;
    return result;
}

Texture copy(Allocator *allocator, Texture texture) {
    Texture result = texture;
    result.filename = copy(allocator, texture.filename);
    return result;
}

void deallocate(Texture texture) {
    deallocate(texture.filename);
}

enum class Texture_Type {
    ALBEDO, METALNESS, ROUGHNESS
};

struct Material {
    String name;
    uint32 use_texture_flags;
    
    String albedo_texture_name;
    Vec3 albedo_color;

    String metalness_texture_name;
    real32 metalness;

    String roughness_texture_name;
    real32 roughness;
    
    //real32 gloss;
    //Vec4 color_override;
    //bool32 use_color_override;
};

Material make_material(String name) {
    // fill with default values
    Material result = {};
    
    result.name = name;
    result.albedo_color = make_vec3(0.5f, 0.5f, 0.5f);
    result.metalness = 0.0f;
    result.roughness = 0.5f;
    
    return result;
}

Material copy(Allocator *allocator, Material material) {
    Material result = material;
    result.name = copy(allocator, material.name);
    result.albedo_texture_name = copy(allocator, material.albedo_texture_name);
    result.metalness_texture_name = copy(allocator, material.metalness_texture_name);
    result.roughness_texture_name = copy(allocator, material.roughness_texture_name);
    return result;
}

void deallocate(Material material) {
    deallocate(material.name);
}

struct Asset_Manager {
    Allocator *allocator_pointer;
    
    Hash_Table<int32, Mesh> mesh_table;
    Hash_Table<int32, Material> material_table;
    Hash_Table<String, Texture> texture_table;

    Hash_Table<String, File_Data> font_file_table;
    Hash_Table<int32, Font> font_table;
};

#endif
