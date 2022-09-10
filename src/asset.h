#ifndef ASSET_H
#define ASSET_H

#include "math.h"
#include "string.h"

#define MAX_TOKEN_TEXT_SIZE   1024
#define NUM_MESH_BUCKETS      128
#define NUM_TEXTURE_BUCKETS   128
#define NUM_MATERIAL_BUCKETS  128
#define NUM_FONT_BUCKETS      128
#define NUM_FONT_FILE_BUCKETS 128

#define MATERIAL_USE_ALBEDO_TEXTURE    (1 << 0)
#define MATERIAL_USE_METALNESS_TEXTURE (1 << 1)
#define MATERIAL_USE_ROUGHNESS_TEXTURE (1 << 2)

// NOTE: RENDERING type is only for OpenGL meshes (added in OpenGL code)
enum class Mesh_Type     { NONE, LEVEL, PRIMITIVE, ENGINE, RENDERING };
enum class Texture_Type  { NONE, LEVEL, DEFAULT };

// TODO: we need more types of Mesh objects. since not all meshes require all the data. for example, a nav mesh
//       doesn't need UVs. and something like a rock mesh won't need joint data. we will also need to modify
//       our mesh loading filetype and loading to acommodate these different types.
struct Mesh {
    Mesh_Type type;
    String name;
    String filename;

    Allocator *allocator;
    
    Mesh *table_prev;
    Mesh *table_next;
    
    real32 *data;
    uint32 num_vertices;
    // size of data in bytes
    uint32 data_size;
    
    // number of components in a vertex
    uint32 n_vertex;
    uint32 n_normal;
    uint32 n_uv;
    
    // NOTE: vertex_stride = total amount of numbers for a single vertex, i.e. sum up all mesh.n_* members
    uint32 vertex_stride;

    uint32 *indices;
    uint32 num_triangles;
    uint32 indices_size;

    AABB aabb;

    bool32 is_loaded;
    bool32 is_double_sided;
};

void deallocate(Mesh *mesh) {
    deallocate(mesh->name);
    deallocate(mesh->filename);
    deallocate(mesh->allocator, mesh->data);
    deallocate(mesh->allocator, mesh->indices);
}

struct Texture {
    Texture_Type type;
    
    String name;
    String filename;
    
    bool32 is_loaded;
    bool32 should_unload;

    Texture *table_next;
    Texture *table_prev;
};

void deallocate(Texture *texture) {
    deallocate(texture->name);
    deallocate(texture->filename);
}

struct Font_File {
    char *filename;
    File_Data file_data;
    Font_File *table_next;
    Font_File *table_prev;
};

struct Font {
    String name;
    Font *table_next;
    Font *table_prev;
    
    File_Data file_data;
    stbtt_fontinfo font_info;
    stbtt_bakedchar *cdata;
    real32 height_pixels;
    real32 scale_for_pixel_height;
    int32 ascent;
    int32 descent;
    int32 line_gap;
    int32 texture_width;
    int32 texture_height;
    int32 first_char;
    int32 num_chars;
    bool32 is_baked;
};

// we don't store whether or not a material has a certain texture. we just store a default texture.
// this is because even when a user defined texture is set, that file can disappear. we don't want to
// have to change some flag when a texture is no longer valid or have to maintain two states like
// not having a texture at all or a texture being invalid.
//
// this decisions also makes it fine to not validate level_info after reading a level file. we just
// set the textures to the given strings. if it doesn't exist, then we just put an ugly, conspicuous,
// default texture.
//
// how do we store these strings then though? we shouldn't use pointers to strings that are already
// allocated. we should allocate new strings. this makes sense. the strings should be used as
// identifiers. if we're just using pointers, to strings already allocated, we wouldn't we just
// store pointers to the entire material? if we delete a material, we need to also delete the pointers.
// if we just keep a string, then even if we delete materials, we can keep that name, but then add a
// new material with the same name and all the materials would be updated.
#define MATERIAL_FIELDS                         \
    String name;                                \
                                                \
    uint32 flags;                               \
                                                \
    String albedo_texture_name;                 \
    Vec3 albedo_color;                          \
                                                \
    String metalness_texture_name;              \
    real32 metalness;                           \
                                                \
    String roughness_texture_name;              \
    real32 roughness;                           \

struct Material {
    MATERIAL_FIELDS
    
    Material *table_prev;
    Material *table_next;
};

void deallocate(Material *material) {
    deallocate(material->name);
}

struct Asset_Manager {
    Allocator *allocator;

    Mesh      *mesh_table[NUM_MESH_BUCKETS];
    Texture   *texture_table[NUM_TEXTURE_BUCKETS];
    Material  *material_table[NUM_MATERIAL_BUCKETS];
    Font      *font_table[NUM_FONT_BUCKETS];
    Font_File *font_file_table[NUM_FONT_FILE_BUCKETS]; // for caching font files

    bool32 gpu_should_unload_level_assets;
};

real32 get_width(Font font, char *text);

#endif
