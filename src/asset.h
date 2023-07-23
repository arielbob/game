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

// material flags
#define MATERIAL_USE_ALBEDO_TEXTURE    (1 << 0)
#define MATERIAL_USE_METALNESS_TEXTURE (1 << 1)
#define MATERIAL_USE_ROUGHNESS_TEXTURE (1 << 2)

// LEVEL is for meshes specifically loaded in for a level by the user or by a level file.
// PRIMITIVE is for meshes you can use in levels, but can't be deleted. also used for default meshes when a
// mesh can't be found or an entity is just created in the editor.
// ENGINE is for rendering code or for the editor, like glyph quads and gizmo arrows.
enum class Mesh_Type     { NONE, LEVEL, PRIMITIVE, ENGINE };
enum class Texture_Type  { NONE, LEVEL, DEFAULT, ENGINE };
enum class Material_Type  { NONE, LEVEL, DEFAULT };

// TODO: we need more types of Mesh objects. since not all meshes require all the data. for example, a nav mesh
//       doesn't need UVs. and something like a rock mesh won't need joint data. we will also need to modify
//       our mesh loading filetype and loading to acommodate these different types.
struct Mesh {
    int32 id;
    
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

    bool32 is_double_sided;
};

void deallocate(Mesh *mesh) {
    deallocate(mesh->name);
    deallocate(mesh->filename);
    deallocate(mesh->allocator, mesh->data);
    deallocate(mesh->allocator, mesh->indices);
}

struct Texture {
    int32 id;
    
    Texture_Type type;
    String name;
    String filename;
    
    Texture *table_next;
    Texture *table_prev;
};

void deallocate(Texture *texture) {
    deallocate(texture->name);
    deallocate(texture->filename);
}

// we have separate structs for Font_Files because a single font file can have multiple
// baked versions, for ex: bold, regular, different sizes, etc.
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
    uint8 *bitmap;
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
// this decision also makes it fine to not validate level_info after reading a level file. we just
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
    Material_Type type;                         \
    int32 id;                                   \
    String name;                                \
                                                \
    uint32 flags;                               \
                                                \
    int32 albedo_texture_id;                    \
    Vec3 albedo_color;                          \
                                                \
    int32 metalness_texture_id;                 \
    real32 metalness;                           \
                                                \
    int32 roughness_texture_id;                 \
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
    int32     total_meshes_added_ever;
    
    Texture   *texture_table[NUM_TEXTURE_BUCKETS];
    int32      total_textures_added_ever;
    
    Material  *material_table[NUM_MATERIAL_BUCKETS];
    int32     total_materials_added_ever;
    
    Font      *font_table[NUM_FONT_BUCKETS];
    Font_File *font_file_table[NUM_FONT_FILE_BUCKETS]; // for caching font files

    bool32 gpu_should_unload_level_assets;
};

real32 get_width(Font font, char *text);

#endif
