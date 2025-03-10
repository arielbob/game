#ifndef ASSET_H
#define ASSET_H

#include "math.h"
#include "string.h"
#include "animation.h"

#define MAX_TOKEN_TEXT_SIZE   1024
#define NUM_TABLE_BUCKETS 128
#define MAX_ASSET_UPDATES 64

#define MAX_BONE_INDICES 4

// material flags
#define MATERIAL_USE_ALBEDO_TEXTURE    (1 << 0)
#define MATERIAL_USE_METALNESS_TEXTURE (1 << 1)
#define MATERIAL_USE_ROUGHNESS_TEXTURE (1 << 2)

// engine meshes
#define ENGINE_QUAD_MESH_ID             -1
#define ENGINE_CIRCLE_MESH_ID           -2
#define ENGINE_CAPSULE_CYLINDER_MESH_ID -3
#define ENGINE_FRAMEBUFFER_QUAD_MESH_ID -4
#define ENGINE_GLYPH_QUAD_MESH_ID       -5
#define ENGINE_CAPSULE_CAP_MESH_ID      -6
#define ENGINE_GIZMO_ARROW_MESH_ID      -7
#define ENGINE_GIZMO_RING_MESH_ID       -8
#define ENGINE_GIZMO_SPHERE_MESH_ID     -9
#define ENGINE_GIZMO_CUBE_MESH_ID       -10
#define ENGINE_TRIANGLE_MESH_ID         -11
#define ENGINE_DEBUG_LINE_MESH_ID       -12
#define ENGINE_DEFAULT_CUBE_MESH_ID     -13
#define ENGINE_DEFAULT_SKINNED_CUBE_MESH_ID -14
#define ENGINE_DEFAULT_PLAYER_CAPSULE_MESH_ID -15

// engine textures
#define ENGINE_DEBUG_TEXTURE_ID             0
#define ENGINE_LIGHTBULB_TEXTURE_ID         -1
#define ENGINE_EDITOR_DOWN_ARROW_TEXTURE_ID -2
#define ENGINE_EDITOR_CHECK_TEXTURE_ID      -3
#define ENGINE_SUN_TEXTURE_ID               -4

// engine cube maps
#define ENGINE_DEFAULT_SKYBOX_CUBE_MAP_ID -1

typedef void (*Handle_Asset_Update_Callback)(String filename);

enum class Asset_Type {
    NONE,
    MESH,
    TEXTURE,
    ANIMATION
};

// LEVEL is for meshes specifically loaded in for a level by the user or by a level file.
// PRIMITIVE is for meshes you can use in levels, but can't be deleted. also used for default meshes when a
// mesh can't be found or an entity is just created in the editor.
// ENGINE is for rendering code or for the editor, like glyph quads and gizmo arrows.
enum class Mesh_Type {
    NONE = 0,
    LEVEL = 1 << 0,
    PRIMITIVE = 1 << 1,
    ENGINE = 1 << 2
};

int32 operator&(Mesh_Type& a, Mesh_Type& b) {
    return ((int) a & (int) b);
}

Mesh_Type operator|(Mesh_Type a, Mesh_Type b) {
    return (Mesh_Type) ((int32) a | (int32) b);
}

enum class Texture_Type { NONE, LEVEL, DEFAULT, ENGINE };
enum class Material_Type  { NONE, LEVEL, DEFAULT };
enum class Cube_Map_Type { NONE, LEVEL, DEFAULT };

// TODO: we need more types of Mesh objects. since not all meshes require all the data. for example, a nav mesh
//       doesn't need UVs. and something like a rock mesh won't need joint data. we will also need to modify
//       our mesh loading filetype and loading to acommodate these different types.
struct Mesh {
    int32 id;
    
    Mesh_Type type;
    String name;
    String filename;
    bool32 is_skinned;

    Allocator *allocator;
    
    Mesh *table_prev;
    Mesh *table_next;
    
    real32 *data;
    uint32 num_vertices;
    // size of data in bytes
    uint32 data_size;

    // skeletons are only associated with a single mesh.
    // you can have many entities with the same mesh. they'd all be using the same
    // mesh object, so we wouldn't really be duplicating anything (neither mesh nor
    // skeleton)
    Skeleton *skeleton;
    
    // number of components in a vertex
    uint32 n_vertex;
    uint32 n_normal;
    uint32 n_uv;
    uint32 n_bone_indices;
    uint32 n_bone_weights;
    
    // NOTE: vertex_stride = total amount of numbers for a single vertex, i.e. sum up all mesh.n_* members
    uint32 vertex_stride;

    uint32 *indices;
    uint32 num_triangles;
    uint32 indices_size;

    AABB aabb;

    bool32 is_double_sided;

    int32 watcher_id;
};

/*
- each mesh watches their directory
- multiple meshes can be in the same directory
- if we add one and the directory AND the callback are the same,
  then we just increment it
- i think we should just allow anything in the directory watching code
  - i.e. if we request to add a folder, we do it; we don't check if it exists already
- we can just leave that logic out of the directory watching code
  - for meshes, it's a given that they all use the same update callback
 */

// this is to keep track all the watchers on the same path that also use
// the same update callback.
// if you need to watch the same directory, but with a different callback,
// you can do that fine on the platform watcher, but you shouldn't use this struct.
struct Directory_Watcher {
    Allocator *allocator;
    String path;
    Directory_Watcher *next;
    int32 id;
    int32 num_watchers;
};

void deallocate(Mesh *mesh) {
    deallocate(mesh->name);
    deallocate(mesh->filename);
    deallocate(mesh->allocator, mesh->data);
    deallocate(mesh->allocator, mesh->indices);

    if (mesh->skeleton) {
        deallocate(mesh->skeleton);
        deallocate(mesh->allocator, mesh->skeleton);
    }
}

struct Cube_Map {
    int32 id;
    Cube_Map_Type type;

    String name;
    String filenames[6];

    Cube_Map *table_next;
    Cube_Map *table_prev;
};

struct Texture {
    int32 id;
    
    Texture_Type type;
    String name;
    String filename;

    int32 watcher_id;

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

enum Asset_Update_Type {
    ASSET_UPDATE_NONE,
    ASSET_UPDATE_MODIFIED,
    ASSET_UPDATE_FILENAME_RENAMED
};

struct Asset_Update_Filename_Change {
    String old_filename;
    String new_filename;
};

struct Asset_Update {
    Asset_Update_Type type;
    union {
        String filename;
        Asset_Update_Filename_Change filename_change;
    };
};

// cleared every frame; can only add, no delete
struct Asset_Update_Queue {
    Platform_Critical_Section critical_section;
    Arena_Allocator arena; // should be accessed in critical_section, cleared end of frame
    
    Asset_Update updates[MAX_ASSET_UPDATES];
    int32 num_updates;
};

struct Asset_Manager {
    Allocator *allocator;

    Mesh      *mesh_table[NUM_TABLE_BUCKETS];
    int32     total_meshes_added_ever;
    Directory_Watcher *mesh_dir_watchers;
    Asset_Update_Queue mesh_update_queue;

    Texture   *texture_table[NUM_TABLE_BUCKETS];
    int32      total_textures_added_ever;
    Directory_Watcher *texture_dir_watchers;
    Asset_Update_Queue texture_update_queue;

    Cube_Map  *cube_map_table[NUM_TABLE_BUCKETS];
    int32     total_cube_maps_added_ever;
    
    Material  *material_table[NUM_TABLE_BUCKETS];
    int32     total_materials_added_ever;

    Skeletal_Animation *animation_table[NUM_TABLE_BUCKETS];
    int32 total_animations_added_ever;
    Directory_Watcher *animation_dir_watchers;
    Asset_Update_Queue animation_update_queue;
    
    Font      *font_table[NUM_TABLE_BUCKETS];
    Font_File *font_file_table[NUM_TABLE_BUCKETS]; // for caching font files

    bool32 gpu_should_unload_level_assets;
};

real32 get_width(Font font, char *text);
bool32 mesh_exists(String name);
void delete_mesh_no_replace(int32 id);
Mesh *add_mesh(String name, String filename, Mesh_Type type, int32 id = -1);
inline Mesh *add_mesh(char *name, char *filename, Mesh_Type type, int32 id = -1);
bool32 refresh_mesh(Mesh *mesh, bool32 ignore_file_in_use = false);
bool32 generate_asset_name(Allocator *allocator, char *asset_type, int32 max_attempts, int32 buffer_size,
                           String *result, bool32 (*exists_fn)(String));
Texture *get_texture_by_path(String filename);
bool32 refresh_texture(Texture *texture);

Skeletal_Animation *get_animation_by_path(String filename);
bool32 refresh_animation(Skeletal_Animation *animation, bool32 ignore_file_in_use = false);
void delete_animation_no_replace(int32 id);
Skeletal_Animation *add_animation(Skeletal_Animation *loaded_animation, int32 id = -1);

#endif
