#include "platform.h"
#include "math.h"
#include "hash_table.h"
#include "game_gl.h"
#include "mesh.h"

// TODO (done): draw other 2D primitives: boxes, lines, etc
// TODO (done): 2D drawing functions using pixel position instead of percentages
// TODO (done): draw text
// TODO (done): mesh loading
// TODO (done): mesh rendering
// TODO (done): basic ui buttons
// TODO (done): mesh picking (ray vs triangle, convert cursor to ray)
// TODO (done): blender randomly crashes sometimes when running the export script, which is also causing issues
//              with the exported normals.
//              update: see the comment above the vertices_list assignment in game_export_2.py for more info.
// TODO (done): fix ray vs triangle test not working sometimes
// TODO (done): basic quaternions
// TODO (done): draw translation gizmo
// TODO (done): move entities using translation gizmo
// TODO (done): draw rotation gizmo
// TODO (done): rotate entities using rotation gizmo
// TODO (done): scale gizmo based on camera distance from gizmo, so that the gizmo stays big and clickable on screen
// TODO (done): textures
// TODO (done): add the Entity struct with the shared Entity info for easier entity access.
// TODO (done): actually send point_light_entity data to shaders (read multiple lights article on learnopengl.com)
// TODO (done): point lights
//              we don't need to do PBR right now - we can just do basic blinn-phong shading
// TODO (done): point light attenuation
// TODO (done): fix entity picking not working when entities are overlapping
//              (try light overlapping plane - the plane gets selected when you click on the light.)
//              this actually had to do with scaling and us not converting t_min back to world_space in
//              ray_intersects_mesh().
// TODO (done): use push buffer for UI elements
// TODO (done): switch to 0,0 in top left coordinate system for screen-space drawing
// TODO (done): first person camera movement
// TODO (done): disable hovering buttons when in freecam mode

// TODO: nicer UI (start with window to display selected entity properties)
// TODO: be able to get font metrics from game code (will have to init fonts in game.cpp, with game_gl.cpp
//       just holding the texture_ids for that font)
// TODO: make game_state, controller_state, and memory global variables
// TODO: material editing (material structs?)
// TODO: level saving/loading
// TODO: directional light (sun light)
// TODO: better level editing (mesh libraries, textures libraries)
// TODO: be able to edit materials
// TODO: in-game console for outputting messages
// TODO: memory alignment in allocate procedures and in ui push buffer
// TODO: add remove key procedure for hash tables.
//       we should use open addressing, which is just storing all the values in an array instead of using linked lists.
//       this is more cache-friendly. this also makes it easier to remove elements without fragmenting memory.
//       we may need to think about ways to have the tables dynamically resize. this would require the allocator
//       to be able to free arbitrary chunks of memory, so we could use something like a free list. or we can just
//       always store enough.
// TODO: make it so you can specify the size of the hash table. (do this after replacing the hash table linked lists with arrays)


// TODO: maybe use a push buffer for entities? and use an Entity_Type enum to differentiate between entities?
//       the upside is that we don't waste space when the amount of one entity type far exceeds another entity
//       type.
//       but the thing is is that we often do need to do operations concerning only a single type of entity.
//       and that could become more complicated if we use a push buffer.
//       not using a push buffer is very annoying when trying to access entities by index. you have to constantly
//       check entity type and then access the correct array.
//       we could do a combination. we could create an Entity struct which contains all the shared fields.
//       then just add a get_selected_entity procedure that just returns an Entity struct. if you need more
//       specific details, then you can check entity.entity_type and cast it to the correct object.
// TODO: be able to draw debug lines
// TODO: window resize handling (recreate framebuffer, modify display_output)

// TODO: typing in text box
// TODO: game should have different Entity structs that have platform-independent data
//       that is then used by game_gl to render that data. for example: Text_Entity, which
//       is just some text with a font name and the game renders that.
//       there should be different make_x_entity procedures that accept that required data for that entity.
//       the entity shaders are handled by game_gl.cpp. all game does is say that some types of entities exist
//       and it is game_gl.cpp's job to render them.
//       this may make us want to have some time of push buffer for all entity types.
//       the alternative is to just have fixed size arrays for each entity type. the upside to that is that it's
//       less complex and most likely faster. the downside is that if the amounts of entity types that exist at
//       a given time differ by a large amount, you'll end up with a lot of unused memory.
// TODO: interface for loading meshes with file explorer
// TODO: nicer button rendering (center the text)
// TODO: undoing

// TODO: create an entity list
// TODO: create a mesh list
// TODO: replace entity and mesh arrays in game_state with free lists

/*
you select an entity
some information of the entity is displayed
- position
- rotation
- entity name
- mesh name
 */

/*
This uses a left-handed coordinate system: positive x is right, positive y is up, positive z is into the screen.
Use your left hand to figure out the direction of the cross product of two vectors.
In 2D, (0, 0) is at the bottom left, positive x is right, positive y is up.
 */

GL_Mesh make_gl_mesh(uint32 vao, uint32 vbo, uint32 num_triangles) {
    GL_Mesh gl_mesh = { vao, vbo };
    return gl_mesh;
}

uint32 gl_create_shader(char *shader_source, uint32 shader_source_size, Shader_Type shader_type) {
    GLenum type = 0;
    if (shader_type == VERTEX) {
        type = GL_VERTEX_SHADER;
    } else if (shader_type == FRAGMENT) {
        type = GL_FRAGMENT_SHADER;
    }

    uint32 shader_id = glCreateShader(type);
    glShaderSource(shader_id, 1, (const char **) &shader_source, (int32 *) &shader_source_size);
    glCompileShader(shader_id);

    int32 success;
    char buffer[256];
    char error_buffer[256];
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    glGetShaderInfoLog(shader_id, 256, NULL, error_buffer);
    if (!success) {
        snprintf(buffer, 256, "%s\n", error_buffer);
        debug_print(error_buffer);
        assert(success);
    }
    
    return shader_id;
}

uint32 gl_compile_and_link_shaders(char *vertex_shader_source, uint32 vertex_shader_source_size,
                                   char *fragment_shader_source, uint32 fragment_shader_source_size) {
    uint32 vertex_shader_id = gl_create_shader(vertex_shader_source, vertex_shader_source_size, VERTEX);
    uint32 fragment_shader_id = gl_create_shader(fragment_shader_source, fragment_shader_source_size, FRAGMENT);
    
    uint32 gl_program_id = glCreateProgram();
    glAttachShader(gl_program_id, vertex_shader_id);
    glAttachShader(gl_program_id, fragment_shader_id);
    glLinkProgram(gl_program_id);
    
    int32 success;
    char error_buffer[256];
    glGetProgramiv(gl_program_id, GL_LINK_STATUS, &success);
    glGetProgramInfoLog(gl_program_id, 256, NULL, error_buffer);
    if (!success) {
        snprintf(error_buffer, 256, "%s\n", error_buffer);
        debug_print(error_buffer);
        assert(success);
    }

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    return gl_program_id;
}

// TODO: should these be inline?
inline void gl_set_uniform_mat4(uint32 shader_id, char* uniform_name, Mat4 *m) {
    int32 uniform_location = glGetUniformLocation(shader_id, uniform_name);
    assert(uniform_location > -1);
    glUniformMatrix4fv(uniform_location, 1, GL_FALSE, (real32 *) m);
}

inline void gl_set_uniform_vec3(uint32 shader_id, char* uniform_name, Vec3 *v) {
    int32 uniform_location = glGetUniformLocation(shader_id, uniform_name);
    assert(uniform_location > -1);
    glUniform3fv(uniform_location, 1, (real32 *) v);
}

inline void gl_set_uniform_vec4(uint32 shader_id, char* uniform_name, Vec4 *v) {
    int32 uniform_location = glGetUniformLocation(shader_id, uniform_name);
    assert(uniform_location > -1);
    glUniform4fv(uniform_location, 1, (real32 *) v);
}

inline void gl_set_uniform_int(uint32 shader_id, char* uniform_name, int32 i) {
    int32 uniform_location = glGetUniformLocation(shader_id, uniform_name);
    assert(uniform_location > -1);
    glUniform1i(uniform_location, i);
}

void gl_load_shader(GL_State *gl_state, Memory *memory,
                      char *vertex_shader_filename, char *fragment_shader_filename, char *shader_name) {
    Marker m = begin_region(memory);

    // NOTE: vertex shader
    Platform_File platform_file;
    bool32 file_exists = platform_open_file(vertex_shader_filename, &platform_file);
    assert(file_exists);

    File_Data vertex_shader_file_data = {};
    vertex_shader_file_data.contents = (char *) region_push(memory, platform_file.file_size);
    bool32 result = platform_read_file(platform_file, &vertex_shader_file_data);
    assert(result);

    platform_close_file(platform_file);

    // NOTE: fragment shader
    file_exists = platform_open_file(fragment_shader_filename, &platform_file);
    assert(file_exists);

    File_Data fragment_shader_file_data = {};
    fragment_shader_file_data.contents = (char *) region_push(memory, platform_file.file_size);
    result = platform_read_file(platform_file, &fragment_shader_file_data);
    assert(result);

    uint32 shader_id = gl_compile_and_link_shaders(vertex_shader_file_data.contents,
                                                   vertex_shader_file_data.size,
                                                   fragment_shader_file_data.contents,
                                                   fragment_shader_file_data.size);

    end_region(memory, m);

    hash_table_add(&gl_state->shader_ids_table, make_string(shader_name), shader_id);
}

void gl_load_texture(GL_State *gl_state, Memory *memory,
                     char *texture_filename, char *texture_name) {
    Marker m = begin_region(memory);
    File_Data texture_file_data = platform_open_and_read_file((Allocator *) &memory->global_stack,
                                                              texture_filename);

    int32 width, height, num_channels;
    stbi_set_flip_vertically_on_load(true);
    uint8 *data = stbi_load_from_memory((uint8 *) texture_file_data.contents, texture_file_data.size,
                                        &width, &height, &num_channels, 0);
    assert(data);
    
    uint32 texture_id;
    glGenTextures(1, &texture_id);

    // TODO: we may want to be able to modify these parameters
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    end_region(memory, m);

    GL_Texture gl_texture = { texture_id, width, height, num_channels };
    hash_table_add(&gl_state->texture_table, make_string(texture_name), gl_texture);
}

// TODO: use the better stb_truetype packing procedures
void gl_init_font(GL_State *gl_state, Memory *memory,
                  char *font_filename, char *font_name,
                  real32 font_height_pixels,
                  int32 font_texture_width, int32 font_texture_height) {
    GL_Font gl_font;
    stbtt_fontinfo font_info;

    File_Data font_file_data;
    String font_filename_string = make_string(font_filename);
    if (!hash_table_find(gl_state->font_file_table, font_filename_string, &font_file_data)) {
        font_file_data = platform_open_and_read_file((Allocator *) &memory->font_arena,
                                                     font_filename);
        hash_table_add(&gl_state->font_file_table, font_filename_string, font_file_data);
    }

    // get font info
    // NOTE: this assumes that the TTF file only has a single font and is at index 0, or else
    //       stbtt_GetFontOffsetForIndex will return a negative value.
    // NOTE: font_info uses the raw data from the file contents, so the file data allocation should NOT
    //       be temporary.
    stbtt_InitFont(&font_info, (uint8 *) font_file_data.contents,
                   stbtt_GetFontOffsetForIndex((uint8 *) font_file_data.contents, 0));
    gl_font.scale_for_pixel_height = stbtt_ScaleForPixelHeight(&font_info, font_height_pixels);
    stbtt_GetFontVMetrics(&font_info, &gl_font.ascent, &gl_font.descent, &gl_font.line_gap);
    gl_font.font_info = font_info;

    uint32 baked_texture_id;
    int32 first_char = 32;
    int32 num_chars = 96;
    gl_font.cdata = (stbtt_bakedchar *) arena_push(&memory->font_arena, 96 * sizeof(stbtt_bakedchar), false);

    Marker m = begin_region(memory);
    uint8 *temp_bitmap = (uint8 *) region_push(&memory->global_stack, font_texture_width*font_texture_height);
    // NOTE: no guarantee that the bitmap will fit the font, so choose temp_bitmap dimensions carefully
    // TODO: we may want to maybe render this out to an image so that we can verify that the font fits
    int32 result = stbtt_BakeFontBitmap((uint8 *) font_file_data.contents, 0,
                                        font_height_pixels, temp_bitmap, font_texture_width, font_texture_height,
                                        first_char, num_chars,
                                        gl_font.cdata);
    assert(result > 0);

    glGenTextures(1, &baked_texture_id);
    glBindTexture(GL_TEXTURE_2D, baked_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
                 font_texture_width, font_texture_height,
                 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    end_region(memory, m);

    gl_font.baked_texture_id = baked_texture_id;
    gl_font.font_height_pixels = font_height_pixels;
    hash_table_add(&gl_state->font_table, make_string(font_name), gl_font);
}

GL_Mesh gl_load_mesh(GL_State *gl_state, Mesh mesh) {
    uint32 vao, vbo, ebo;
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
        
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.data_size, mesh.data, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices_size, mesh.indices, GL_STATIC_DRAW);

    // vertices
    glVertexAttribPointer(0, mesh.n_vertex, GL_FLOAT, GL_FALSE,
                          mesh.vertex_stride * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    
    // normals
    glVertexAttribPointer(1, mesh.n_normal, GL_FLOAT, GL_FALSE,
                          mesh.vertex_stride * sizeof(real32),
                          (void *) (mesh.n_vertex * sizeof(real32)));
    glEnableVertexAttribArray(1);

    // UVs
    glVertexAttribPointer(2, mesh.n_uv, GL_FLOAT, GL_FALSE,
                          mesh.vertex_stride * sizeof(real32),
                          (void *) ((mesh.n_vertex + mesh.n_normal) * sizeof(real32)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    GL_Mesh gl_mesh = { vao, vbo, mesh.num_triangles };
    return gl_mesh;
}

uint32 gl_use_shader(GL_State *gl_state, char *shader_name) {
    uint32 shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string(shader_name), &shader_id);
    assert(shader_exists);
    glUseProgram(shader_id);
    return shader_id;
}

void gl_use_texture(GL_State *gl_state, char *texture_name) {
    // TODO: will have to add parameter to specify which texture slot to use
    GL_Texture texture;
    uint32 texture_exists = hash_table_find(gl_state->texture_table, make_string(texture_name), &texture);
    assert(texture_exists);
    glBindTexture(GL_TEXTURE_2D, texture.id);
}

GL_Mesh gl_use_mesh(GL_State *gl_state, char *mesh_name) {
    GL_Mesh gl_mesh;
    uint32 mesh_exists = hash_table_find(gl_state->mesh_table, make_string(mesh_name), &gl_mesh);
    assert(mesh_exists);
    glBindVertexArray(gl_mesh.vao);
    return gl_mesh;
}

void gl_draw_solid_mesh(GL_State *gl_state, Render_State *render_state,
                        char *mesh_name, char *texture_name, 
                        Vec4 color,
                        Transform transform,
                        bool32 use_color_override) {
    uint32 shader_id = gl_use_shader(gl_state, "solid");
    GL_Mesh gl_mesh = gl_use_mesh(gl_state, mesh_name);

    if (!use_color_override && !texture_name) assert(!"No texture name provided.");
    if (!use_color_override) gl_use_texture(gl_state, texture_name);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);
    gl_set_uniform_int(shader_id, "use_color_override", use_color_override);
    
    glDrawElements(GL_TRIANGLES, gl_mesh.num_triangles * 3, GL_UNSIGNED_INT, 0);

    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_solid_color_mesh(GL_State *gl_state, Render_State *render_state,
                              char *mesh_name,
                              Vec4 color,
                              Transform transform) {
    gl_draw_solid_mesh(gl_state, render_state,
                       mesh_name, NULL, 
                       color,
                       transform,
                       true);
}

void gl_draw_mesh(GL_State *gl_state, Game_State *game_state,
                  char *mesh_name, char *texture_name,
                  Vec4 color,
                  Transform transform,
                  bool32 use_color_override) {
    Render_State *render_state = &game_state->render_state;

    uint32 shader_id = gl_use_shader(gl_state, "basic_3d");

    GL_Mesh gl_mesh = gl_use_mesh(gl_state, mesh_name);

    if (!use_color_override && !texture_name) assert(!"No texture name provided.");
    if (!use_color_override) gl_use_texture(gl_state, texture_name);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    // NOTE: we may need to think about this for transparent materials
    Vec3 material_color = truncate_v4_to_v3(color);
    gl_set_uniform_vec3(shader_id, "material_color", &material_color);
    
    Vec3 light_color = make_vec3(1.0f, 1.0f, 1.0f);
    Vec3 light_position = make_vec3(0.0f, 1.0f, 0.0f);
    //gl_set_uniform_vec3(shader_id, "light_color", &light_color);
    //gl_set_uniform_vec3(shader_id, "light_pos", &light_position);
    
    gl_set_uniform_vec3(shader_id, "camera_pos", &render_state->camera.position);
    gl_set_uniform_int(shader_id, "use_color_override", use_color_override);

    glDrawElements(GL_TRIANGLES, gl_mesh.num_triangles * 3, GL_UNSIGNED_INT, 0);

    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_textured_mesh(GL_State *gl_state, Render_State *render_state,
                           char *mesh_name, char *texture_name, Transform transform) {
    uint32 shader_id = gl_use_shader(gl_state, "basic_3d_textured");
    gl_use_texture(gl_state, texture_name);

    GL_Mesh gl_mesh;
    uint32 mesh_exists = hash_table_find(gl_state->mesh_table, make_string(mesh_name), &gl_mesh);
    assert(mesh_exists);
    glBindVertexArray(gl_mesh.vao);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);

    glDrawElements(GL_TRIANGLES, gl_mesh.num_triangles * 3, GL_UNSIGNED_INT, 0);

    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void gl_draw_wireframe(GL_State *gl_state, Render_State *render_state,
                       char *mesh_name, Transform transform) {
    uint32 shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("debug_wireframe"), &shader_id);
    assert(shader_exists);
    glUseProgram(shader_id);

    GL_Mesh gl_mesh;
    uint32 mesh_exists = hash_table_find(gl_state->mesh_table, make_string(mesh_name), &gl_mesh);
    assert(mesh_exists);
    glBindVertexArray(gl_mesh.vao);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, gl_mesh.num_triangles * 3, GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUseProgram(0);
    glBindVertexArray(0);
}

void copy_aligned_quad_to_arrays(stbtt_aligned_quad q, real32 *vertices, real32 *uvs) {
    vertices[0] = q.x0;
    vertices[1] = q.y0;
    vertices[2] = q.x1;
    vertices[3] = q.y0;
    vertices[4] = q.x1;
    vertices[5] = q.y1;
    vertices[6] = q.x0;
    vertices[7] = q.y1;

    uvs[0] = q.s0;
    uvs[1] = q.t0;
    uvs[2] = q.s1;
    uvs[3] = q.t0;
    uvs[4] = q.s1;
    uvs[5] = q.t1;
    uvs[6] = q.s0;
    uvs[7] = q.t1;
}

// TODO: we may want to reconsider our 2D coordinate-space and maybe have y=0 start at the top and increase
//       going downwards. this is because right now if we wanted to draw some text starting from the top
//       and going downwards, we would have to first compute the height of the text, then draw it at
//       window_height - text_height. although.. the same can be said if we were to use a y=0 at top
//       coordinate-system for drawing text from the bottom. actually, that's not necessarily true. text
//       always grows downwards, so the benefit of starting at the top is that we can always at least see
//       the start of the text. idk.
void gl_draw_text(GL_State *gl_state,
                  Display_Output display_output,
                  char *font_name,
                  real32 x_pos_pixels, real32 y_pos_pixels,
                  char *text, Vec3 color) {
    uint32 text_shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("text"), &text_shader_id);
    assert(shader_exists);
    glUseProgram(text_shader_id);

    GL_Mesh glyph_mesh;
    uint32 mesh_exists = hash_table_find(gl_state->debug_mesh_table, make_string("glyph_quad"), &glyph_mesh);
    assert(mesh_exists);
    glBindVertexArray(glyph_mesh.vao);

    GL_Font font;
    uint32 font_exists = hash_table_find(gl_state->font_table, make_string(font_name), &font);
    assert(font_exists);

    // TODO: we don't actually need to use a Mat4 here; we can just use a Mat2 and set z = 0
    Mat4 ortho_clip_matrix = make_ortho_clip_matrix((real32) display_output.width,
                                                    (real32) display_output.height,
                                                    0.0f, 100.0f);
    gl_set_uniform_mat4(text_shader_id, "cpv_matrix", &ortho_clip_matrix);
    gl_set_uniform_vec3(text_shader_id, "color", &color);

    // NOTE: we disable depth test so that overlapping characters such as the "o" in "fo" doesn't cover the
    //       quad of the previous character, causing a cut off look.
    // NOTE: we assume that GL_DEPTH_TEST is disabled
    //glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, font.baked_texture_id);
    glBindBuffer(GL_ARRAY_BUFFER, glyph_mesh.vbo);

    real32 quad_vertices[8];
    real32 quad_uvs[8];
    real32 line_advance = font.scale_for_pixel_height * (font.ascent - font.descent + font.line_gap);
    real32 start_x_pos_pixels = x_pos_pixels;
    while (*text) {
        if (*text >= 32 && *text < 128 || *text == '-') {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font.cdata, 512, 512, *text - 32, &x_pos_pixels, &y_pos_pixels, &q, 1);

            copy_aligned_quad_to_arrays(q, quad_vertices, quad_uvs);

            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
            glBufferSubData(GL_ARRAY_BUFFER, (int *) sizeof(quad_vertices), sizeof(quad_uvs), quad_uvs);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        } else if (*text == '\n') {
            x_pos_pixels = start_x_pos_pixels;
            y_pos_pixels += line_advance;
        }
        
        ++text;
    }

    //glEnable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

GL_Framebuffer gl_make_framebuffer(int32 width, int32 height) {
    GL_Framebuffer framebuffer;

    glGenFramebuffers(1, &framebuffer.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

    // color buffer texture
    glGenTextures(1, &framebuffer.color_buffer_texture);
    glBindTexture(GL_TEXTURE_2D, framebuffer.color_buffer_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 width, height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           framebuffer.color_buffer_texture, 0);

    // render buffer (depth only)
    glGenRenderbuffers(1, &framebuffer.render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, framebuffer.render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer.render_buffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        debug_print("Framebuffer is not complete.\n");
        assert(false);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return framebuffer;
}

void gl_delete_framebuffer(GL_Framebuffer framebuffer) {
    // delete framebuffer, color buffer, render buffer
    glDeleteFramebuffers(1, &framebuffer.fbo);
    glDeleteTextures(1, &framebuffer.color_buffer_texture);
    glDeleteRenderbuffers(1, &framebuffer.render_buffer);
}

void generate_circle_vertices(real32 *buffer, int32 num_vertices, real32 radius) {
    assert(num_vertices > 0);
    // generate circle vertices on the yz-plane, i.e. x always = 0.
    for (int32 i = 0; i < num_vertices; i++) {
        real32 t = (real32) i / num_vertices;
        real32 angle = 2.0f * PI * t;
        real32 x = cosf(angle) * radius;
        real32 y = sinf(angle) * radius;
        buffer[i * 3] = 0.0f;
        buffer[i * 3 + 1] = y;
        buffer[i * 3 + 2] = x;
    }
}

void gl_init(Memory *memory, GL_State *gl_state, Display_Output display_output) {
    gl_state->shader_ids_table = make_hash_table<uint32>((Allocator *) &memory->hash_table_stack);
    gl_state->debug_mesh_table = make_hash_table<GL_Mesh>((Allocator *) &memory->hash_table_stack);
    gl_state->font_table = make_hash_table<GL_Font>((Allocator *) &memory->hash_table_stack);
    gl_state->mesh_table = make_hash_table<GL_Mesh>((Allocator *) &memory->hash_table_stack);
    gl_state->texture_table = make_hash_table<GL_Texture>((Allocator *) &memory->hash_table_stack);
    gl_state->font_file_table = make_hash_table<File_Data>((Allocator *) &memory->hash_table_stack);

    uint32 vbo, vao, ebo;

    // NOTE: triangle mesh
    real32 triangle_vertices[] = {
        0.0f, 0.0f, 0.0f,
        0.5f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    hash_table_add(&gl_state->debug_mesh_table, make_string("triangle"), make_gl_mesh(vao, vbo, 1));

    // NOTE: square mesh
    real32 square_vertices[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
    };
    uint32 square_indices[] = {
        0, 1, 2,
        1, 3, 2
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices), square_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(square_indices), square_indices, GL_STATIC_DRAW); 

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    hash_table_add(&gl_state->debug_mesh_table, make_string("square"), make_gl_mesh(vao, vbo, 2));

    // NOTE: framebuffer mesh
    real32 framebuffer_mesh_data[] = {
        // positions  uvs
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,

         1.0f, 1.0f,  1.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(framebuffer_mesh_data), framebuffer_mesh_data, GL_STATIC_DRAW);

    // positions
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    // uvs
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(real32), (void *) (2 * sizeof(real32)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    hash_table_add(&gl_state->debug_mesh_table, make_string("framebuffer_quad"), make_gl_mesh(vao, vbo, 2));

    // NOTE: glyph quad
    // we store them separately like this because we use glBufferSubData to send the vertex positions
    // directly to the shader, and i don't think there's a way to easily modify interleaved data, unless
    // you're modifying all of the data, but when we modify the data we only modify the positions and not the uvs.
#if 1
    // the values in these arrays don't matter; we're just filling these arrays up with enough values such that
    // sizeof() gives the correct values
    real32 quad_vertices[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };
    real32 quad_uvs[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };
#endif
    uint32 quad_indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices) + sizeof(quad_uvs), 0, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, (int *) sizeof(quad_vertices), sizeof(quad_uvs), quad_uvs);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW); 

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          2 * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          2 * sizeof(real32), (void *) sizeof(quad_vertices));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    hash_table_add(&gl_state->debug_mesh_table, make_string("glyph_quad"), make_gl_mesh(vao, vbo, 2));

    // NOTE: line mesh
    real32 line_vertices[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    hash_table_add(&gl_state->debug_mesh_table, make_string("line"), make_gl_mesh(vao, vbo, 0));

    real32 circle_vertices[32*3];
    generate_circle_vertices(circle_vertices, 32, 1.0f);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle_vertices), circle_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(real32), (void *) 0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    hash_table_add(&gl_state->debug_mesh_table, make_string("circle"), make_gl_mesh(vao, vbo, 0));

    // NOTE: shaders
    gl_load_shader(gl_state, memory,
                   "src/shaders/basic.vs", "src/shaders/basic.fs", "basic");
    gl_load_shader(gl_state, memory,
                   "src/shaders/text.vs", "src/shaders/text.fs", "text");
    gl_load_shader(gl_state, memory,
                   "src/shaders/solid.vs", "src/shaders/solid.fs", "solid");
    gl_load_shader(gl_state, memory,
                   "src/shaders/basic_3d.vs", "src/shaders/basic_3d.fs", "basic_3d");

    gl_load_shader(gl_state, memory,
                   "src/shaders/basic_3d_textured.vs", "src/shaders/basic_3d_textured.fs", "basic_3d_textured");
    gl_load_shader(gl_state, memory,
                   "src/shaders/debug_wireframe.vs",
                   "src/shaders/debug_wireframe.fs",
                   "debug_wireframe");
    gl_load_shader(gl_state, memory,
                   "src/shaders/framebuffer.vs", "src/shaders/framebuffer.fs",
                   "framebuffer");
    gl_state->gizmo_framebuffer = gl_make_framebuffer(display_output.width, display_output.height);

    glGenBuffers(1, &gl_state->global_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, gl_state->global_ubo);
    // TODO: not sure if 1024 bytes is enough
    // we add 1 to MAX_POINT_LIGHTS for the int to hold num_point_lights.
    // maybe we could put num_point lights at the end of the uniform buffer object?
    glBufferData(GL_UNIFORM_BUFFER, (MAX_POINT_LIGHTS + 1) * sizeof(GL_Point_Light), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //glBindBufferRange(GL_UNIFORM_BUFFER, 0, gl_state->global_ubo, 0, 4);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, gl_state->global_ubo);
    GLenum error = glGetError();

    uint32 shader_id = gl_use_shader(gl_state, "basic_3d");
    error = glGetError();
    uint32 uniform_block_index = glGetUniformBlockIndex(shader_id, "shader_globals");
    error = glGetError();
    glUniformBlockBinding(shader_id, uniform_block_index, 0);
    glUseProgram(0);
    error = glGetError();

    // NOTE: fonts
    gl_init_font(gl_state, memory, "c:/windows/fonts/times.ttf", "times32", 32.0f, 512, 512);
    gl_init_font(gl_state, memory, "c:/windows/fonts/times.ttf", "times24", 24.0f, 512, 512);
    gl_init_font(gl_state, memory, "c:/windows/fonts/cour.ttf", "courier24", 24.0f, 512, 512);
    gl_init_font(gl_state, memory, "c:/windows/fonts/cour.ttf", "courier18", 18.0f, 512, 512);
    gl_init_font(gl_state, memory, "c:/windows/fonts/courbd.ttf", "courier18b", 18.0f, 512, 512);

    // NOTE: textures
    gl_load_texture(gl_state, memory, "src/textures/debug_texture.bmp", "debug");

    // NOTE: disable culling for now, just for easier debugging...
#if 0
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
#endif

    glEnable(GL_DEPTH_TEST);  
}

// NOTE: This draws a triangle that has its bottom left corner at position.
//       Position is based on percentages, so 50% x and 50%y would put the bottom left corner of the triangle
//       in the middle of the screen.
void gl_draw_triangle_p(GL_State *gl_state,
                        Display_Output display_output, Vec2 position,
                        real32 width_pixels, real32 height_pixels,
                        Vec4 color) {
    uint32 basic_shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("basic"), &basic_shader_id);
    assert(shader_exists);
    glUseProgram(basic_shader_id);

    GL_Mesh triangle_mesh;
    bool32 mesh_exists = hash_table_find(gl_state->debug_mesh_table, make_string("triangle"), &triangle_mesh);
    assert(mesh_exists);
    glBindVertexArray(triangle_mesh.vao);

    Vec2 clip_space_position = make_vec2(position.x * 2.0f - 1.0f,
                                         position.y * -2.0f + 1.0f);
    
    real32 clip_space_width  = width_pixels / (display_output.width / 2.0f);
    real32 clip_space_height = height_pixels / (display_output.height / 2.0f);

    Mat4 model_matrix = (make_translate_matrix(make_vec3(clip_space_position, 0.0f)) *
                         make_scale_matrix(y_axis, clip_space_height) *
                         make_scale_matrix(x_axis, clip_space_width));
    gl_set_uniform_mat4(basic_shader_id, "model", &model_matrix);

    gl_set_uniform_vec4(basic_shader_id, "color", &color);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_triangle(GL_State *gl_state,
                      Display_Output display_output,
                      real32 x_pos_pixels, real32 y_pos_pixels,
                      real32 width_pixels, real32 height_pixels,
                      Vec4 color) {
    // TODO: this might be wrong with the new screen-space coordinate-system (0, 0) in top left
    gl_draw_triangle_p(gl_state, display_output,
                       make_vec2(x_pos_pixels / display_output.width, y_pos_pixels / display_output.height),
                       width_pixels, height_pixels,
                       color);
}

void gl_draw_line_p(GL_State *gl_state,
                    Display_Output display_output,
                    Vec2 start, Vec2 end,
                    Vec4 color) {
    uint32 basic_shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("basic"), &basic_shader_id);
    assert(shader_exists);
    glUseProgram(basic_shader_id);

    GL_Mesh line_mesh;
    uint32 mesh_exists = hash_table_find(gl_state->debug_mesh_table, make_string("line"), &line_mesh);
    assert(mesh_exists);
    glBindVertexArray(line_mesh.vao);

    Vec2 clip_space_start = make_vec2(start.x * 2.0f - 1.0f,
                                      start.y * -2.0f + 1.0f);
    Vec2 clip_space_end = make_vec2(end.x * 2.0f - 1.0f,
                                    end.y * -2.0f + 1.0f);
    Vec2 clip_space_length = clip_space_end - clip_space_start;

    Mat4 model_matrix = (make_translate_matrix(make_vec3(clip_space_start, 0.0f)) *
                         make_scale_matrix(y_axis, clip_space_length.y) *
                         make_scale_matrix(x_axis, clip_space_length.x));
    gl_set_uniform_mat4(basic_shader_id, "model", &model_matrix);

    gl_set_uniform_vec4(basic_shader_id, "color", &color);

    glDrawArrays(GL_LINES, 0, 3);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_line(GL_State *gl_state,
                  Display_Output display_output,
                  Vec2 start_pixels, Vec2 end_pixels,
                  Vec4 color) {
    uint32 basic_shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("basic"), &basic_shader_id);
    assert(shader_exists);
    glUseProgram(basic_shader_id);

    GL_Mesh line_mesh;
    uint32 vao_exists = hash_table_find(gl_state->debug_mesh_table, make_string("line"), &line_mesh);
    assert(vao_exists);
    glBindVertexArray(line_mesh.vao);

    Vec2 clip_space_start = make_vec2(start_pixels.x / display_output.width * 2.0f - 1.0f,
                                      start_pixels.y / display_output.height * -2.0f + 1.0f);
    Vec2 clip_space_end = make_vec2(end_pixels.x / display_output.width * 2.0f - 1.0f,
                                    end_pixels.y / display_output.height * -2.0f + 1.0f);
    Vec2 clip_space_length = clip_space_end - clip_space_start;

    Mat4 model_matrix = (make_translate_matrix(make_vec3(clip_space_start, 0.0f)) *
                         make_scale_matrix(y_axis, clip_space_length.y) *
                         make_scale_matrix(x_axis, clip_space_length.x));
    gl_set_uniform_mat4(basic_shader_id, "model", &model_matrix);

    gl_set_uniform_vec4(basic_shader_id, "color", &color);

    glDrawArrays(GL_LINES, 0, 3);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_line(GL_State *gl_state,
                  Display_Output display_output,
                  Vec2 start_pixels, Vec2 end_pixels,
                  Vec3 color) {
    gl_draw_line(gl_state, display_output, start_pixels, end_pixels, make_vec4(color, 1.0f));
}

// NOTE: percentage based position
void gl_draw_quad_p(GL_State *gl_state,
                    Display_Output display_output,
                    real32 x, real32 y,
                    real32 width_pixels, real32 height_pixels,
                    Vec4 color) {
    uint32 basic_shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("basic"), &basic_shader_id);
    assert(shader_exists);
    glUseProgram(basic_shader_id);

    GL_Mesh square_mesh;
    uint32 mesh_exists = hash_table_find(gl_state->debug_mesh_table, make_string("square"), &square_mesh);
    assert(mesh_exists);
    glBindVertexArray(square_mesh.vao);

    Vec2 clip_space_position = make_vec2(x * 2.0f - 1.0f,
                                         y * -2.0f + 1.0f);
    
    real32 clip_space_width  = width_pixels / (display_output.width / 2.0f);
    real32 clip_space_height = height_pixels / (display_output.height / 2.0f);

    Mat4 model_matrix = (make_translate_matrix(make_vec3(clip_space_position, 0.0f)) *
                         make_scale_matrix(y_axis, clip_space_height) *
                         make_scale_matrix(x_axis, clip_space_width));
    gl_set_uniform_mat4(basic_shader_id, "model", &model_matrix);

    gl_set_uniform_vec4(basic_shader_id, "color", &color);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

// NOTE: pixel based position, with (0,0) being at bottom left and (width, height) being at top left
void gl_draw_quad(GL_State *gl_state,
                  Display_Output display_output,
                  real32 x_pos_pixels, real32 y_pos_pixels,
                  real32 width_pixels, real32 height_pixels,
                  Vec4 color) {
    uint32 basic_shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("basic"), &basic_shader_id);
    assert(shader_exists);
    glUseProgram(basic_shader_id);

    GL_Mesh square_mesh;
    uint32 mesh_exists = hash_table_find(gl_state->debug_mesh_table, make_string("square"), &square_mesh);
    assert(mesh_exists);
    glBindVertexArray(square_mesh.vao);

    Vec2 clip_space_position = make_vec2((x_pos_pixels / display_output.width) * 2.0f - 1.0f,
                                         (y_pos_pixels / display_output.height) * -2.0f + 1.0f);
    
    real32 clip_space_width  = width_pixels / (display_output.width / 2.0f);
    real32 clip_space_height = height_pixels / (display_output.height / 2.0f);

    Mat4 model_matrix = (make_translate_matrix(make_vec3(clip_space_position, 0.0f)) *
                         make_scale_matrix(y_axis, clip_space_height) *
                         make_scale_matrix(x_axis, clip_space_width));
    gl_set_uniform_mat4(basic_shader_id, "model", &model_matrix);

    gl_set_uniform_vec4(basic_shader_id, "color", &color);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_quad(GL_State *gl_state,
                  Display_Output display_output,
                  real32 x_pos_pixels, real32 y_pos_pixels,
                  real32 width_pixels, real32 height_pixels,
                  Vec3 color) {
    gl_draw_quad(gl_state, display_output, x_pos_pixels, y_pos_pixels, width_pixels, height_pixels,
                 make_vec4(color, 1.0f));
}

void draw_sound_cursor(GL_State *gl_state,
                       Display_Output display_output, Win32_Sound_Output *win32_sound_output,
                       real32 cursor_position, Vec3 color) {
    real32 cursor_width = 10.0f;
    real32 cursor_x = ((cursor_position *
                        display_output.width) - cursor_width / 2.0f);
    real32 cursor_height = 20.0f;
    gl_draw_triangle(gl_state, display_output,
                     cursor_x, display_output.height - 202 - cursor_height,
                     cursor_width, cursor_height,
                     make_vec4(color, 1.0f));

    gl_draw_line(gl_state, display_output,
                 make_vec2(cursor_position * display_output.width, display_output.height - 202.0f),
                 make_vec2(cursor_position * display_output.width, (real32) display_output.height),
                 color);
}

void draw_sound_buffer(GL_State *gl_state,
                       Display_Output display_output, Win32_Sound_Output *win32_sound_output) {
    int32 max_samples = win32_sound_output->buffer_size / win32_sound_output->bytes_per_sample;

    real32 channel_height = 100.0;
    real32 height_offset = channel_height;
    gl_draw_quad(gl_state, display_output,
                 0.0f, display_output.height - height_offset,
                 (real32) display_output.width, channel_height,
                 make_vec3(0.1f, 0.1f, 0.1f));
    gl_draw_line(gl_state, display_output,
                 make_vec2(0.0f, display_output.height - height_offset - 1),
                 make_vec2((real32) display_output.width, display_output.height - height_offset - 1),
                 make_vec3(1.0f, 1.0f, 1.0f));

    height_offset += channel_height + 1;
    gl_draw_quad(gl_state, display_output,
                 0.0f, display_output.height - height_offset,
                 (real32) display_output.width, channel_height,
                 make_vec3(0.1f, 0.1f, 0.1f));
    gl_draw_line(gl_state, display_output,
                 make_vec2(0.0f, display_output.height - height_offset - 1),
                 make_vec2((real32) display_output.width, display_output.height - height_offset - 1),
                 make_vec3(1.0f, 1.0f, 1.0f));

    int32 increment = max_samples / display_output.width;
    for (int32 i = 0; i < max_samples; i += increment) {
        real32 sample_x = (real32) i / max_samples * display_output.width;

        int16 left_sample = win32_sound_output->accumulated_sound_buffer[2*i];
        int16 right_sample = win32_sound_output->accumulated_sound_buffer[2*i + 1];

        real32 sample_height = (real32) left_sample / 32768 * channel_height;
        real32 midline_offset = display_output.height - channel_height / 2.0f;

        gl_draw_line(gl_state, display_output,
                     make_vec2(sample_x, midline_offset),
                     make_vec2(sample_x, midline_offset - sample_height),
                     make_vec3(0.0f, 1.0f, 0.0f));

        
        sample_height = (real32) right_sample / 32768 * channel_height;
        midline_offset -= channel_height + 1;

        gl_draw_line(gl_state, display_output,
                     make_vec2(sample_x, midline_offset),
                     make_vec2(sample_x, midline_offset - sample_height),
                     make_vec3(0.0f, 1.0f, 0.0f));

    }

    real32 play_cursor_position = (real32) win32_sound_output->current_play_cursor / win32_sound_output->buffer_size;
    draw_sound_cursor(gl_state, display_output, win32_sound_output, play_cursor_position, make_vec3(1.0f, 1.0f, 1.0f));
    real32 write_cursor_position = (real32) win32_sound_output->current_write_cursor / win32_sound_output->buffer_size;
    draw_sound_cursor(gl_state, display_output, win32_sound_output, write_cursor_position, make_vec3(1.0f, 0.0f, 0.0f));
}

real32 get_width(GL_State *gl_state, char *font_name, char *text) {
    GL_Font font;
    bool32 font_exists = hash_table_find(gl_state->font_table, make_string(font_name), &font);
    assert(font_exists);

    real32 width = 0;

    while (*text) {
        int32 advance, left_side_bearing;
        stbtt_GetCodepointHMetrics(&font.font_info, *text, &advance, &left_side_bearing);
        width += (advance) * font.scale_for_pixel_height;
        
        if (*(text + 1)) {
            width += font.scale_for_pixel_height * stbtt_GetCodepointKernAdvance(&font.font_info,
                                                                                 *text, *(text + 1));
        }

        text++;
    }
    
    return width;
}

void gl_draw_ui_text(GL_State *gl_state, Display_Output display_output, UI_Manager *ui_manager, UI_Text ui_text) {
    UI_Text_Style style = ui_text.style;

    if (style.use_offset_shadow) {
        gl_draw_text(gl_state, display_output, ui_text.font,
                     ui_text.x, ui_text.y + 2.0f,
                     ui_text.text, style.offset_shadow_color);
    }

    gl_draw_text(gl_state, display_output, ui_text.font,
                 ui_text.x, ui_text.y,
                 ui_text.text, style.color);
}

void gl_draw_ui_text_button(GL_State *gl_state, Display_Output display_output,
                            UI_Manager *ui_manager, UI_Text_Button ui_text_button) {
    Vec3 color = make_vec3(1.0f, 1.0f, 1.0f);

    if (ui_id_equals(ui_manager->hot, ui_text_button.id)) {
        color = make_vec3(0.0f, 1.0f, 0.0f);
        if (ui_id_equals(ui_manager->active, ui_text_button.id)) {
            color = make_vec3(0.0f, 0.0f, 1.0f);
        }
    } else {
        color = make_vec3(1.0f, 0.0f, 0.0f);
    }

    gl_draw_quad(gl_state, display_output, ui_text_button.x, ui_text_button.y,
                 ui_text_button.width, ui_text_button.height, color);

    // TODO: center this.. will have to use font metrics
    gl_draw_text(gl_state, display_output, ui_text_button.font,
                 ui_text_button.x, ui_text_button.y + ui_text_button.height,
                 ui_text_button.text, make_vec3(1.0f, 1.0f, 1.0f));
}

void gl_draw_ui_text_box(GL_State *gl_state, Display_Output display_output,
                         UI_Manager *ui_manager, UI_Text_Box text_box) {
        Vec3 color = make_vec3(1.0f, 1.0f, 1.0f);

        if (ui_id_equals(ui_manager->active, text_box.id)) {
            color = make_vec3(0.0f, 0.0f, 1.0f);
        } else if (ui_id_equals(ui_manager->hot, text_box.id)) {
            color = make_vec3(0.0f, 1.0f, 0.0f);
        } else {
            color = make_vec3(1.0f, 0.0f, 0.0f);
        }

        UI_Text_Box_Style style = text_box.style;
        gl_draw_quad(gl_state, display_output, text_box.x, text_box.y,
                     style.width + style.padding_x * 2, style.height + style.padding_y * 2,
                     color);

        glEnable(GL_SCISSOR_TEST);
        // TODO: should move where the text renders depending on if the cursor moves outside of the
        //       text box's bounds.
        // glScissor((int32) (text_box.x + style.padding_x), (int32) (display_output.height - (text_box.y + style.padding_y)),
        //           (int32) style.width, (int32) style.height);
        glScissor((int32) (text_box.x + style.padding_x), (int32) (display_output.height - text_box.y - style.height - style.padding_y),
                  (int32) style.width, (int32) style.height);
        gl_draw_text(gl_state, display_output, style.font,
                     text_box.x + style.padding_x, text_box.y + style.height - style.padding_y,
                     text_box.current_text, make_vec3(1.0f, 1.0f, 1.0f));
        glDisable(GL_SCISSOR_TEST);

        if (ui_id_equals(ui_manager->active, text_box.id)) {
            // TODO: this cursor should actually be calculated using focus_cursor_index. we need to
            //       split the text string on that index and draw the cursor at the width of the left
            //       split. when we draw it, it has to be offset if it is outside the bounds of the text
            //       box.
            // in focus
            real32 text_width = get_width(gl_state, style.font, text_box.current_text);
            gl_draw_quad(gl_state, display_output,
                         text_box.x + text_width + style.padding_x, text_box.y + style.padding_y,
                         12.0f, style.height, make_vec3(0.0f, 1.0f, 0.0f));
        }
}

void gl_draw_ui_box(GL_State *gl_state, Display_Output display_output,
                    UI_Manager *ui_manager, UI_Box box) {
    UI_Box_Style style = box.style;

    gl_draw_quad(gl_state, display_output,
                 box.x, box.y,
                 style.width, style.height, style.background_color);
}

void gl_draw_ui_line(GL_State *gl_state, Display_Output display_output,
                     UI_Manager *ui_manager, UI_Line line) {
    UI_Line_Style style = line.style;

    // TODO: use style.line_width in gl_draw_line()
    gl_draw_line(gl_state, display_output,
                 line.start, line.end,
                 style.color);
}

// TODO: we could, along with gl_draw_quad, replace the model_matrix stuff with just updating the VBO.
//       the issue with this is that it could make it harder for us to do more interesting transformations like
//       rotation.
void gl_draw_ui(GL_State *gl_state, UI_Manager *ui_manager, Display_Output display_output) {
    UI_Push_Buffer *push_buffer = &ui_manager->push_buffer;
    uint8 *address = (uint8 *) push_buffer->base;

    while (address < ((uint8 *) push_buffer->base + push_buffer->used)) {
        UI_Element *element = (UI_Element *) address;
        switch (element->type) {
            case UI_TEXT: {
                UI_Text *ui_text = (UI_Text *) element;
                gl_draw_ui_text(gl_state, display_output, ui_manager, *ui_text);
                address += sizeof(UI_Text);
            } break;
            case UI_TEXT_BUTTON: {
                UI_Text_Button *ui_text_button = (UI_Text_Button *) element;
                gl_draw_ui_text_button(gl_state, display_output, ui_manager, *ui_text_button);
                address += sizeof(UI_Text_Button);
            } break;
            case UI_TEXT_BOX: {
                UI_Text_Box *ui_text_box = (UI_Text_Box *) element;
                gl_draw_ui_text_box(gl_state, display_output, ui_manager, *ui_text_box);
                address += sizeof(UI_Text_Box);
            } break;
            case UI_BOX: {
                UI_Box *ui_box = (UI_Box *) element;
                gl_draw_ui_box(gl_state, display_output, ui_manager, *ui_box);
                address += sizeof(UI_Box);
            } break;
            case UI_LINE: {
                UI_Line *ui_line = (UI_Line *) element;
                gl_draw_ui_line(gl_state, display_output, ui_manager, *ui_line);
                address += sizeof(UI_Line);
            } break;
            default: {
                assert(!"Unhandled UI element type.");
            }
        }
    }

#if 0
    for (int32 i = 0; i < ui_manager->num_buttons; i++) {
        UI_Button button = ui_manager->buttons[i];

        Vec3 color = make_vec3(1.0f, 1.0f, 1.0f);

        if (ui_id_equals(ui_manager->hot, button.id)) {
            color = make_vec3(0.0f, 1.0f, 0.0f);
            if (ui_id_equals(ui_manager->active, button.id)) {
                color = make_vec3(0.0f, 0.0f, 1.0f);
            }
        } else {
            color = make_vec3(1.0f, 0.0f, 0.0f);
        }

        gl_draw_quad(gl_state, display_output, button.x, button.y,
                     button.width, button.height, color);

        // TODO: center this.. will have to use font metrics
        gl_draw_text(gl_state, display_output, button.font,
                     button.x, button.y,
                     button.text, make_vec3(1.0f, 1.0f, 1.0f));
    }

    for (int32 i = 0; i < ui_manager->num_text_boxes; i++) {
        UI_Text_Box text_box = ui_manager->text_boxes[i];

        Vec3 color = make_vec3(1.0f, 1.0f, 1.0f);


        if (ui_id_equals(ui_manager->active, text_box.id)) {
            color = make_vec3(0.0f, 0.0f, 1.0f);
        } else if (ui_id_equals(ui_manager->hot, text_box.id)) {
            color = make_vec3(0.0f, 1.0f, 0.0f);
        } else {
            color = make_vec3(1.0f, 0.0f, 0.0f);
        }

        UI_Text_Box_Style style = text_box.style;
        gl_draw_quad(gl_state, display_output, text_box.x, text_box.y,
                     style.width + style.padding_x * 2, style.height + style.padding_y * 2,
                     color);

        glEnable(GL_SCISSOR_TEST);
        // TODO: should move where the text renders depending on if the cursor moves outside of the
        //       text box's bounds.
        glScissor((int32) (text_box.x + style.padding_x), (int32) (text_box.y + style.padding_y),
                  (int32) style.width, (int32) style.height);
        gl_draw_text(gl_state, display_output, style.font,
                     text_box.x + style.padding_x, text_box.y + style.padding_y,
                     text_box.current_text, make_vec3(1.0f, 1.0f, 1.0f));
        glDisable(GL_SCISSOR_TEST);

        if (ui_id_equals(ui_manager->active, text_box.id)) {
            // TODO: this cursor should actually be calculated using focus_cursor_index. we need to
            //       split the text string on that index and draw the cursor at the width of the left
            //       split. when we draw it, it has to be offset if it is outside the bounds of the text
            //       box.
            // in focus
            real32 text_width = get_width(gl_state, style.font, text_box.current_text);
            gl_draw_quad(gl_state, display_output,
                         text_box.x + text_width + style.padding_x, text_box.y + style.padding_y,
                         12.0f, style.height, make_vec3(0.0f, 1.0f, 0.0f));
        }
    }

    for (int32 i = 0; i < ui_manager->num_texts; i++) {
        UI_Text ui_text = ui_manager->texts[i];

        Vec3 color = make_vec3(1.0f, 1.0f, 1.0f);

        // TODO: center this.. will have to use font metrics
        gl_draw_text(gl_state, display_output, ui_text.font,
                     ui_text.x, ui_text.y,
                     ui_text.text, color);
    }
#endif
}

void gl_draw_circle(GL_State *gl_state, Render_State *render_state, Transform transform, Vec4 color) {
    uint32 shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("basic_3d"), &shader_id);
    assert(shader_exists);
    glUseProgram(shader_id);

    GL_Mesh circle_mesh;
    uint32 vao_exists = hash_table_find(gl_state->debug_mesh_table, make_string("circle"), &circle_mesh);
    assert(vao_exists);
    glBindVertexArray(circle_mesh.vao);

    Mat4 model_matrix = get_model_matrix(transform);
    gl_set_uniform_mat4(shader_id, "model_matrix", &model_matrix);
    gl_set_uniform_mat4(shader_id, "cpv_matrix", &render_state->cpv_matrix);
    gl_set_uniform_vec4(shader_id, "color", &color);

    glDrawArrays(GL_LINE_LOOP, 0, 32);

    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_draw_gizmo(GL_State *gl_state, Render_State *render_state, Editor_State *editor_state) {
    Transform_Mode transform_mode = editor_state->transform_mode;
    Gizmo gizmo = editor_state->gizmo;

    uint32 shader_id;
    uint32 shader_exists = hash_table_find(gl_state->shader_ids_table, make_string("basic_3d"), &shader_id);
    assert(shader_exists);
    glUseProgram(shader_id);

    Transform x_transform, y_transform, z_transform;

    // this is for a world-space gizmo
    // TODO: if we have the model matrix of the mesh, we could use the columns of that to create the model
    //       matrix of the gizmo when in local mode. but this is fine too.
    if (transform_mode == TRANSFORM_GLOBAL) {
        x_transform = gizmo.transform;
        x_transform.rotation = make_quaternion();
        y_transform = gizmo.transform;
        y_transform.rotation = make_quaternion(90.0f, z_axis);
        z_transform = gizmo.transform;
        z_transform.rotation = make_quaternion(-90.0f, y_axis);
    } else {
        x_transform = gizmo.transform;
        y_transform = gizmo.transform;
        y_transform.rotation = gizmo.transform.rotation*make_quaternion(90.0f, z_axis);
        z_transform = gizmo.transform;
        z_transform.rotation = gizmo.transform.rotation*make_quaternion(-90.0f, y_axis);
    }


    Vec4 x_handle_hover = make_vec4(1.0f, 0.8f, 0.8f, 1.0f);
    Vec4 y_handle_hover = make_vec4(0.8f, 1.0f, 0.8f, 1.0f);
    Vec4 z_handle_hover = make_vec4(0.8f, 0.8f, 1.0f, 1.0f);

    Gizmo_Handle hovered_handle = editor_state->hovered_gizmo_handle;

    Vec4 x_handle_color = make_vec4(x_axis, 1.0f);
    Vec4 y_handle_color = make_vec4(y_axis, 1.0f);
    Vec4 z_handle_color = make_vec4(z_axis, 1.0f);

    // translation arrows
    if (hovered_handle == GIZMO_TRANSLATE_X) {
        x_handle_color = x_handle_hover;
    } else if (hovered_handle == GIZMO_TRANSLATE_Y) {
        y_handle_color = y_handle_hover;
    } else if (hovered_handle == GIZMO_TRANSLATE_Z) {
        z_handle_color = z_handle_hover;
    }

    gl_draw_solid_color_mesh(gl_state, render_state, gizmo.arrow_mesh_name, x_handle_color, x_transform);
    gl_draw_solid_color_mesh(gl_state, render_state, gizmo.arrow_mesh_name, y_handle_color, y_transform);
    gl_draw_solid_color_mesh(gl_state, render_state, gizmo.arrow_mesh_name, z_handle_color, z_transform);

    Transform sphere_mask_transform = gizmo.transform;
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    gl_draw_solid_color_mesh(gl_state, render_state, gizmo.sphere_mesh_name,
                             make_vec4(0.0f, 0.0f, 0.0f, 1.0f), sphere_mask_transform);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // rotation rings
    x_handle_color = make_vec4(x_axis, 1.0f);
    y_handle_color = make_vec4(y_axis, 1.0f);
    z_handle_color = make_vec4(z_axis, 1.0f);
    if (hovered_handle == GIZMO_ROTATE_X) {
        x_handle_color = x_handle_hover;
    } else if (hovered_handle == GIZMO_ROTATE_Y) {
        y_handle_color = y_handle_hover;
    } else if (hovered_handle == GIZMO_ROTATE_Z) {
        z_handle_color = z_handle_hover;
    }

    real32 offset_value = 1e-2f;
    Vec3 offset = make_vec3(offset_value, offset_value, offset_value);
    y_transform.scale += offset;
    z_transform.scale += 2.0f * offset;

    gl_draw_solid_color_mesh(gl_state, render_state, gizmo.ring_mesh_name, x_handle_color, x_transform);
    gl_draw_solid_color_mesh(gl_state, render_state, gizmo.ring_mesh_name, y_handle_color, y_transform);
    gl_draw_solid_color_mesh(gl_state, render_state, gizmo.ring_mesh_name, z_handle_color, z_transform);
}

void gl_draw_framebuffer(GL_State *gl_state, GL_Framebuffer framebuffer) {
    gl_use_shader(gl_state, "framebuffer");

    glBindTexture(GL_TEXTURE_2D, framebuffer.color_buffer_texture);

    GL_Mesh gl_mesh;
    uint32 mesh_exists = hash_table_find(gl_state->debug_mesh_table, make_string("framebuffer_quad"), &gl_mesh);
    assert(mesh_exists);
    glBindVertexArray(gl_mesh.vao);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);
    glBindVertexArray(0);
}

void gl_render(GL_State *gl_state, Controller_State *controller_state, Game_State *game_state,
               Display_Output display_output, Win32_Sound_Output *win32_sound_output) {
    Render_State *render_state = &game_state->render_state;

    for (int32 i = 0; i < game_state->num_meshes; i++) {
        Mesh *mesh = &game_state->meshes[i];
        if (!mesh->is_loaded) {
            if (!hash_table_exists(gl_state->mesh_table, make_string(mesh->name))) {
                GL_Mesh gl_mesh = gl_load_mesh(gl_state, *mesh);
                hash_table_add(&gl_state->mesh_table, mesh->name, gl_mesh);
            } else {
                debug_print("%s already loaded.\n", mesh->name);
            }

            mesh->is_loaded = true;
        }
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLineWidth(1.0f);

    //gl_draw_quad(gl_state, display_output, 0.0f, 0.0f, 50.0f, 50.0f, make_vec3(1.0f, 0.0f, 0.0f));
    //gl_draw_quad_p(gl_state, display_output, 0.5f, 0.5f, 50.0f, 50.0f, make_vec3(1.0f, 0.0f, 0.0f));
    /*gl_draw_text(gl_state, display_output,
                 "times24",
                 0.0f, 32.0f,
                 "Hello, world!", make_vec3(1.0f, 1.0f, 1.0f));*/

    local_persist real32 t = 0.0f;
    t += 0.01f;

    Editor_State *editor_state = &game_state->editor_state;

    // point lights
    for (int32 i = 0; i < game_state->num_point_lights; i++) {
        Point_Light_Entity *entity = &game_state->point_lights[i];
        char *mesh_name = game_state->meshes[entity->mesh_index].name;
        char *texture_name = entity->texture_name;

        gl_draw_solid_mesh(gl_state, render_state, 
                           mesh_name, texture_name,
                           make_vec4(entity->color_override, 1.0f),
                           entity->transform,
                           texture_name == NULL);

        if (game_state->editor_state.selected_entity_type == ENTITY_POINT_LIGHT &&
            game_state->editor_state.selected_entity_index == i) {
            gl_draw_wireframe(gl_state, render_state, mesh_name, entity->transform);
        }
    }

    glBindBuffer(GL_UNIFORM_BUFFER, gl_state->global_ubo);
    int64 ubo_offset = 0;
    glBufferSubData(GL_UNIFORM_BUFFER, (int32 *) ubo_offset, sizeof(int32), &game_state->num_point_lights);
    // NOTE: not sure why we use 16 here, instead of 32, which is the size of the GL_Point_Light struct.
    //       i think we just use the aligned offset of the first member of the struct, which is a vec4, so we offset
    //       by 16 since it's the closest multiple.
    ubo_offset += 16;

    for (int32 i = 0; i < game_state->num_point_lights; i++) {
        // TODO: we may just want to replace position and light_color with vec4s in Point_Light_Entity.
        //       although this would be kind of annoying since we would have to modify the Transform struct.
        GL_Point_Light gl_point_light = {
            make_vec4(game_state->point_lights[i].transform.position, 1.0f),
            make_vec4(game_state->point_lights[i].light_color, 1.0f),
            game_state->point_lights[i].d_min,
            game_state->point_lights[i].d_max,
        };

        glBufferSubData(GL_UNIFORM_BUFFER, (int32 *) ubo_offset,
                        sizeof(GL_Point_Light), &gl_point_light);
        ubo_offset += sizeof(GL_Point_Light) + 8; // add 8 bytes of padding so that it aligns to size of vec4
    }

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // entities
    for (int32 i = 0; i < game_state->num_entities; i++) {
        Normal_Entity *entity = &game_state->entities[i];
        char *mesh_name = game_state->meshes[entity->mesh_index].name;
        char *texture_name = entity->texture_name;

        gl_draw_mesh(gl_state, game_state,
                     mesh_name, texture_name,
                     make_vec4(entity->color_override, 1.0f),
                     entity->transform,
                     texture_name == NULL);

        if (game_state->editor_state.selected_entity_type == ENTITY_NORMAL &&
            game_state->editor_state.selected_entity_index == i) {
            gl_draw_wireframe(gl_state, render_state, mesh_name, entity->transform);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, gl_state->gizmo_framebuffer.fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (editor_state->selected_entity_index >= 0) {
        gl_draw_gizmo(gl_state, render_state, editor_state);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

#if 0
    real32 quad_x_offset = sinf(t) * (50.0f / display_output.width);

    gl_draw_quad_p(gl_state, display_output,
                   0.5f + quad_x_offset, 0.5f,
                   100.0f, 100.0f,
                   make_vec3(0.0f, 1.0f, 0.0f));

    gl_draw_triangle_p(gl_state, display_output,
                     make_vec2(0.5f, 0.5f),
                     100.0f, 100.0f,
                     make_vec3(1.0f, 0.0f, 0.0f));

    real32 square_width_percentage = (100.0f / display_output.width);
    real32 square_height_percentage = (100.0f / display_output.height);
    gl_draw_line_p(gl_state, display_output,
                   make_vec2(0.75f, 0.25f), make_vec2(0.5f + quad_x_offset, 0.5f),
                   make_vec3(1.0f, 1.0f, 1.0f));
    gl_draw_line_p(gl_state, display_output,
                   make_vec2(0.75f, 0.25f), make_vec2(0.5f + quad_x_offset + square_width_percentage, 0.5f),
                   make_vec3(1.0f, 1.0f, 1.0f));
    gl_draw_line_p(gl_state, display_output,
                   make_vec2(0.75f, 0.25f), make_vec2(0.5f + quad_x_offset, 0.5f + square_height_percentage),
                   make_vec3(1.0f, 1.0f, 1.0f));
    gl_draw_line_p(gl_state, display_output,
                   make_vec2(0.75f, 0.25f),
                   make_vec2(0.5f + quad_x_offset + square_width_percentage, 0.5f + square_height_percentage),
                   make_vec3(1.0f, 1.0f, 1.0f));
#endif

    Vec3 text_color = make_vec3(1.0f, 1.0f, 1.0f);

#if 0
    
    gl_draw_text(gl_state, display_output, "times32",
                 200.0f, display_output.height / 3.0f,
                 "In the midst of winter, I found there was, within me, an invincible summer.\n\nAnd that makes me happy. For it says that no matter how hard the world pushes against me,\nwithin me, there's something stronger - something better, pushing right back.", 
                 text_color);
#endif

    // TODO: create a nicer function for this
#if 0
    char buf[128];
    string_format(buf, sizeof(buf), "cursor pos: (%d, %d)",
                  (int32) game_state->cursor_pos.x, (int32) game_state->cursor_pos.y);
    gl_draw_text(gl_state, display_output, "times24",
                 0.0f, 15.0f,
                 buf,
                 text_color);

    string_format(buf, sizeof(buf), "hot: %s", game_state->ui_manager.hot);
    gl_draw_text(gl_state, display_output, "times24",
                 0.0f, 115.0f,
                 buf,
                 text_color);

    string_format(buf, sizeof(buf), "active: %s", game_state->ui_manager.active);
    gl_draw_text(gl_state, display_output, "times24",
                 0.0f, 100.0f,
                 buf,
                 text_color);
#endif

#if 0
    String pressed_chars_string = make_string(buf, 0);
    append_string(&pressed_chars_string, make_string("current pressed chars: "), make_string(""), sizeof(buf));
    for (int32 i = 0; i < controller_state->num_pressed_chars; i++) {
        char c = controller_state->pressed_chars[i];
        char temp_buf[256];
        string_format(temp_buf, sizeof(temp_buf), "%d ", c);

        append_string(&pressed_chars_string, pressed_chars_string, make_string(temp_buf), sizeof(buf));
    }

    char output_buf[256];
    to_char_array(pressed_chars_string, output_buf, sizeof(output_buf));
    gl_draw_text(gl_state, display_output, "times24",
                 0.0f, 200.0f,
                 output_buf,
                 text_color);
#endif

    // NOTE: this looks really broken, since we switched the screen-space coordinate system to have 0,0 at the
    //       top left
    // draw_sound_buffer(gl_state, display_output, win32_sound_output);

/*
    Transform transform = { make_vec3(-.25f, -.25f, -0.25f),
                            t*50.0f, t*50.0f, 0.0f,
                            make_vec3(0.5f, 0.5f, 0.5f) };
*/
    // gl_draw_mesh(gl_state, render_state, "cube", "basic_3d", transform);    

    glDisable(GL_DEPTH_TEST);
    
    gl_draw_framebuffer(gl_state, gl_state->gizmo_framebuffer);

    gl_draw_ui(gl_state, &game_state->ui_manager, display_output);

    glEnable(GL_DEPTH_TEST);
}
