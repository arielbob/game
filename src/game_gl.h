#ifndef GAME_GL_H
#define GAME_GL_H

#include "string.h"

// copy constants from here: https://www.khronos.org/registry/OpenGL/api/GL/glext.h
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_MULTISAMPLE                    0x809D
#define GL_BLEND_DST_RGB                  0x80C8
#define GL_BLEND_SRC_RGB                  0x80C9
#define GL_BLEND_DST_ALPHA                0x80CA
#define GL_BLEND_SRC_ALPHA                0x80CB
#define GL_POINT_FADE_THRESHOLD_SIZE      0x8128
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_MIRRORED_REPEAT                0x8370
#define GL_MAX_TEXTURE_LOD_BIAS           0x84FD
#define GL_TEXTURE_LOD_BIAS               0x8501
#define GL_INCR_WRAP                      0x8507
#define GL_DECR_WRAP                      0x8508
#define GL_TEXTURE_DEPTH_SIZE             0x884A
#define GL_TEXTURE_COMPARE_MODE           0x884C
#define GL_TEXTURE_COMPARE_FUNC           0x884D
#define GL_POINT_SIZE_MIN                 0x8126
#define GL_POINT_SIZE_MAX                 0x8127
#define GL_POINT_DISTANCE_ATTENUATION     0x8129
#define GL_GENERATE_MIPMAP                0x8191
#define GL_GENERATE_MIPMAP_HINT           0x8192
#define GL_FOG_COORDINATE_SOURCE          0x8450
#define GL_FOG_COORDINATE                 0x8451
#define GL_FRAGMENT_DEPTH                 0x8452
#define GL_CURRENT_FOG_COORDINATE         0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE      0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE    0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER   0x8456
#define GL_FOG_COORDINATE_ARRAY           0x8457
#define GL_COLOR_SUM                      0x8458
#define GL_CURRENT_SECONDARY_COLOR        0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE     0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE     0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE   0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER  0x845D
#define GL_SECONDARY_COLOR_ARRAY          0x845E
#define GL_TEXTURE_FILTER_CONTROL         0x8500
#define GL_DEPTH_TEXTURE_MODE             0x884B
#define GL_COMPARE_R_TO_TEXTURE           0x884E
#define GL_BLEND_COLOR                    0x8005
#define GL_BLEND_EQUATION                 0x8009
#define GL_CONSTANT_COLOR                 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR       0x8002
#define GL_CONSTANT_ALPHA                 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA       0x8004
#define GL_FUNC_ADD                       0x8006
#define GL_FUNC_REVERSE_SUBTRACT          0x800B
#define GL_FUNC_SUBTRACT                  0x800A
#define GL_MIN                            0x8007
#define GL_MAX                            0x8008
#define GL_FRAMEBUFFER                    0x8D40
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_COLOR_ATTACHMENT4              0x8CE4
#define GL_COLOR_ATTACHMENT5              0x8CE5
#define GL_COLOR_ATTACHMENT6              0x8CE6
#define GL_COLOR_ATTACHMENT7              0x8CE7
#define GL_COLOR_ATTACHMENT8              0x8CE8
#define GL_COLOR_ATTACHMENT9              0x8CE9
#define GL_COLOR_ATTACHMENT10             0x8CEA
#define GL_COLOR_ATTACHMENT11             0x8CEB
#define GL_COLOR_ATTACHMENT12             0x8CEC
#define GL_COLOR_ATTACHMENT13             0x8CED
#define GL_COLOR_ATTACHMENT14             0x8CEE
#define GL_COLOR_ATTACHMENT15             0x8CEF
#define GL_COLOR_ATTACHMENT16             0x8CF0
#define GL_COLOR_ATTACHMENT17             0x8CF1
#define GL_COLOR_ATTACHMENT18             0x8CF2
#define GL_COLOR_ATTACHMENT19             0x8CF3
#define GL_COLOR_ATTACHMENT20             0x8CF4
#define GL_COLOR_ATTACHMENT21             0x8CF5
#define GL_COLOR_ATTACHMENT22             0x8CF6
#define GL_COLOR_ATTACHMENT23             0x8CF7
#define GL_COLOR_ATTACHMENT24             0x8CF8
#define GL_COLOR_ATTACHMENT25             0x8CF9
#define GL_COLOR_ATTACHMENT26             0x8CFA
#define GL_COLOR_ATTACHMENT27             0x8CFB
#define GL_COLOR_ATTACHMENT28             0x8CFC
#define GL_COLOR_ATTACHMENT29             0x8CFD
#define GL_COLOR_ATTACHMENT30             0x8CFE
#define GL_COLOR_ATTACHMENT31             0x8CFF
#define GL_RENDERBUFFER                   0x8D41
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_SRGB                           0x8C40
#define GL_SRGB_ALPHA                     0x8C42
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B
#define GL_R8                             0x8229
#define GL_R16                            0x822A
#define GL_INVALID_INDEX                  0xFFFFFFFFu
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF

#define GL_HEAP_SIZE MEGABYTES(128)
#define NUM_SHADER_BUCKETS 128

#define TEXT_SHADOW_OFFSET 2.0f
#define NUM_CIRCLE_VERTICES 64
#define NUM_MSAA_SAMPLES 1
#define MAX_ALPHA_MASKS 16
#define MAX_POINT_LIGHTS 16
#define GL_MAX_TEXT_CHARACTERS 1024
#define GLOBAL_UBO_SIZE 1024

#define FRAMEBUFFER_IS_HDR   (1 << 0)
#define FRAMEBUFFER_HAS_ALPHA (1 << 1)

enum Shader_Type {
    VERTEX,
    FRAGMENT
};

struct GL_Mesh {
    Mesh_Type type;
    uint32 vao;
    uint32 vbo;
    uint32 num_triangles;

    String name;
    GL_Mesh *table_prev;
    GL_Mesh *table_next;
};

void deallocate(GL_Mesh *gl_mesh) {
    deallocate(gl_mesh->name);
}

struct GL_Font {
    uint32 baked_texture_id;

    String name; // read-only memory
    GL_Font *table_prev;
    GL_Font *table_next;
};

struct GL_Texture {
    // TODO: not sure if we need to store texture type? same for type in gl_mesh
    Texture_Type type;
    uint32 id; // gl texture id

    int32 width;
    int32 height;
    int32 num_channels;

    String name;
    GL_Texture *table_prev;
    GL_Texture *table_next;
};

void deallocate(GL_Texture *gl_texture) {
    deallocate(gl_texture->name);
}

// TODO: there are different variations of this (like depth + stencil, instead of just depth).
//       will need to make different ones if necessary.
struct GL_Framebuffer {
    uint32 fbo;
    uint32 color_buffer_texture;
    uint32 render_buffer;
    bool32 is_multisampled;
    int32 num_samples;
};

struct GL_Point_Light {
    Vec4 position;
    Vec4 color;
    real32 d_min;
    real32 d_max;
};

struct GL_Shader {
    // this struct includes entire shader program (i.e. vertex + fragment),
    // so don't need to store type here
    uint32 id;

    String name;
    GL_Shader *table_prev;
    GL_Shader *table_next;
};

struct GL_Alpha_Mask_Stack {
    uint32 texture_ids[MAX_ALPHA_MASKS];
    int32 index = -1;
};

struct GL_UI_Data {
    uint32 vao;
    uint32 vbo;
    uint32 ebo;
};

struct GL_State {
    Heap_Allocator heap;
    
    // we just use the same keys across GL and game code. GL rendering meshes should be added in game code
    // with mesh type of Mesh_Type::RENDERING
    GL_Mesh    *mesh_table[NUM_MESH_BUCKETS];
    GL_Texture *texture_table[NUM_TEXTURE_BUCKETS];
    GL_Font    *font_table[NUM_FONT_BUCKETS];
    GL_Shader  *shader_table[NUM_SHADER_BUCKETS];

    // TODO: will have to delete these and remake them on window resize
    GL_Framebuffer gizmo_framebuffer;
    GL_Framebuffer msaa_framebuffer;
    uint32 global_ubo;

    GL_UI_Data ui_data;
    GL_Framebuffer alpha_mask_framebuffer;
    GL_Alpha_Mask_Stack alpha_mask_stack;

    bool32 scissor_enabled;
    Vec2_int32 scissor_position;
    Vec2_int32 scissor_dimensions;
};

#if 0
struct GL_State {
    Hash_Table<String, uint32> shader_ids_table;

    // NOTE: we use separate tables for meshes only used by the OpenGL code and the meshes added by the game
    //       this is because there is a 1:1 correspondence between the keys in game_state.mesh_table and
    //       gl_state.mesh_table. if we were to combine the two, the IDs could collide, since we're just
    //       making IDs based off of a running count kept by the table you're adding to.
    // for GL specific meshes
#if 0
    Hash_Table<int32, GL_Mesh> rendering_mesh_table;
    // for game's common meshes
    Hash_Table<int32, GL_Mesh> common_mesh_table;
    Hash_Table<int32, GL_Mesh> primitive_mesh_table
#endif

    Hash_Table<int32, GL_Mesh> rendering_mesh_table;
    

    // for GL specific textures
    Hash_Table<int32, GL_Texture> rendering_texture_table;

    Hash_Table<int32, uint32> font_texture_table;

    // game assets
    Hash_Table<int32, GL_Mesh> mesh_table;
    Hash_Table<int32, GL_Texture> texture_table;
    
    // TODO: will have to delete these and remake them on window resize
    GL_Framebuffer gizmo_framebuffer;
    GL_Framebuffer msaa_framebuffer;
    uint32 global_ubo;

    GL_Framebuffer alpha_mask_framebuffer;
    GL_Alpha_Mask_Stack alpha_mask_stack;
    
    // TODO: maybe figure out a better way of doing this.
    //       we switched to using IDs instead of strings, but the downside is that we can't just search
    //       tables with a string anymore.
    // TODO: maybe just use strings to find meshes in rendering mesh table by name
    int32 triangle_mesh_id;
    int32 quad_mesh_id;
    int32 framebuffer_quad_mesh_id;
    int32 glyph_quad_mesh_id;
    int32 line_mesh_id;
    int32 circle_mesh_id;
    int32 sphere_mesh_id;
    int32 capsule_cylinder_mesh_id;
    int32 capsule_cap_mesh_id;

    int32 light_icon_texture_id;
};
#endif

#endif
