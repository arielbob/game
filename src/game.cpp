#include "platform.h"
#include "memory.h"
#include "game.h"

global_variable int32 samples_written_2;

void fill_sound_buffer_with_sine_wave(Sound_Output *sound_output, uint32 num_samples) {
    assert(num_samples < sound_output->max_samples); 

    real32 frequency = 252.0f;
    real32 volume = 10000.0f;
    int16 *sound_buffer = sound_output->sound_buffer;

    // NOTE: we use 252.0 is a factor of our sample rate (44100). we want it to be a factor so that we can
    //       cleanly loop around our samples_written variable. this prevents sound artifacts due to floating
    //       point error.
    int32 sample_count_period = (int32) ((1.0f / frequency) * sound_output->samples_per_second);

    for (uint32 i = 0; i < num_samples; i++) {
        real32 x = (real32) samples_written_2 / sound_output->samples_per_second;

        int16 sample = (int16) (volume * sinf(frequency * 2.0f * PI * x));

        *(sound_buffer++) = sample;
        *(sound_buffer++) = sample;
        samples_written_2++;
        samples_written_2 %= sample_count_period;
    }
}

void fill_sound_buffer_with_audio(Sound_Output *sound_output, bool32 is_playing,
                                  Audio_Source *audio,
                                  int32 num_samples) {
    assert((uint32) num_samples < sound_output->max_samples); 
    int16 *sound_buffer = sound_output->sound_buffer;
    int32 i = 0;

    if (is_playing) {
        int16 *music_buffer_at = audio->samples;
        music_buffer_at += audio->current_sample * 2;

        for (; i < num_samples; i++) {
            if (audio->current_sample >= audio->total_samples) {
                if (audio->should_loop) {
                    audio->current_sample = 0;
                    music_buffer_at = audio->samples;
                } else {
                    break;
                }
            }

            *(sound_buffer++) = (int16) (*(music_buffer_at++) * audio->volume);
            *(sound_buffer++) = (int16) (*(music_buffer_at++) * audio->volume);
            audio->current_sample++;
        }
    }

    for (; i < num_samples; i++) {
        *(sound_buffer++) = 0;
        *(sound_buffer++) = 0;
    }
}

Audio_Source make_audio_source(uint32 total_samples, uint32 current_sample,
                               real32 volume, bool32 should_loop,
                               int16 *samples) {
    Audio_Source audio_source;
    audio_source.total_samples = total_samples;
    audio_source.current_sample = current_sample;
    audio_source.volume = volume;
    audio_source.should_loop = should_loop;
    audio_source.samples = samples;

    return audio_source;
}

bool32 int32_equals(int32 a, int32 b) {
    return a == b;
}

Material make_material(String_Buffer material_name,
                       int32 texture_id,
                       real32 specular_exponent,
                       Vec4 color_override, bool32 use_color_override) {
    Material material = { material_name, texture_id,
                          specular_exponent, color_override, use_color_override };
    return material;
}

bool32 material_exists(Game_State *game_state, int32 material_id) {
    return hash_table_exists(game_state->material_table, material_id);
}

Material get_material(Game_State *game_state, int32 material_id) {
    Material material;
    bool32 material_exists = hash_table_find(game_state->material_table,
                                             material_id,
                                             &material);
    assert(material_exists);
    return material;
}

Material *get_material_pointer(Game_State *game_state, int32 material_id) {
    Material *material;
    bool32 material_exists = hash_table_find_pointer(game_state->material_table,
                                                     material_id,
                                                     &material);
    assert(material_exists);
    return material;
}

inline bool32 mesh_exists(Game_State *game_state, int32 mesh_id) {
    return hash_table_exists(game_state->mesh_table, mesh_id);
}

Mesh get_mesh(Game_State *game_state, int32 mesh_id) {
    Mesh mesh;
    bool32 mesh_exists = hash_table_find(game_state->mesh_table,
                                         mesh_id,
                                         &mesh);
    assert(mesh_exists);
    return mesh;
}

Mesh *get_mesh_pointer(Game_State *game_state, int32 mesh_id) {
    Mesh *mesh;
    bool32 mesh_exists = hash_table_find_pointer(game_state->mesh_table,
                                                 mesh_id,
                                                 &mesh);
    assert(mesh_exists);
    return mesh;
}

Texture get_texture(Game_State *game_state, int32 texture_id) {
    assert(texture_id >= 0);
    Texture texture;
    bool32 texture_exists = hash_table_find(game_state->texture_table,
                                            texture_id,
                                            &texture);
    assert(texture_exists);
    return texture;
}

Normal_Entity make_entity(Game_State *game_state,
                          int32 mesh_id,
                          int32 material_id,
                          Transform transform) {
    assert(mesh_exists(game_state, mesh_id));
    assert(material_exists(game_state, material_id));
    
    Normal_Entity entity = { ENTITY_NORMAL, transform,
                             mesh_id, material_id };
    return entity;
}

Texture make_texture(Game_State *game_state, String_Buffer texture_name, String_Buffer filename) {
    Texture texture = {};
    texture.name = texture_name;
    texture.filename = filename;
    return texture;
}

Point_Light_Entity make_point_light_entity(Game_State *game_state,
                                           int32 mesh_id,
                                           int32 material_id,
                                           Vec3 light_color,
                                           real32 d_min, real32 d_max,
                                           Transform transform) {
    assert(mesh_exists(game_state, mesh_id));
    assert(material_exists(game_state, material_id));
    Point_Light_Entity entity = { ENTITY_POINT_LIGHT, transform,
                                  mesh_id, material_id,
                                  light_color, d_min, d_max };
    return entity;
}

int32 add_mesh(Game_State *game_state, Mesh mesh) {
    int32 mesh_id = game_state->mesh_table.total_added_ever;
    hash_table_add(&game_state->mesh_table, mesh_id, mesh);
    return mesh_id;
}

void add_entity(Game_State *game_state, Normal_Entity entity) {
    assert(game_state->num_normal_entities < MAX_ENTITIES);
    game_state->normal_entities[game_state->num_normal_entities++] = entity;
}

void add_point_light_entity(Game_State *game_state, Point_Light_Entity entity) {
    assert(game_state->num_point_lights < MAX_POINT_LIGHTS);
    game_state->point_lights[game_state->num_point_lights++] = entity;
}

int32 add_material(Game_State *game_state, Material material) {
    int32 material_id = game_state->material_table.total_added_ever;
    hash_table_add(&game_state->material_table, material_id, material);
    return material_id;
}

void add_font(Game_State *game_state, Font font) {
    hash_table_add(&game_state->font_table, make_string(font.name), font);
}

int32 add_texture(Game_State *game_state, Texture texture) {
    int32 texture_id = game_state->texture_table.total_added_ever;
    hash_table_add(&game_state->texture_table, texture_id, texture);
    return texture_id;
}

Font get_font(Game_State *game_state, char *font_name) {
    Font font;
    bool32 font_exists = hash_table_find(game_state->font_table, make_string(font_name), &font);
    assert(font_exists);
    return font;
}

void init_camera(Camera *camera, Display_Output *display_output) {
    camera->position = make_vec3(0.0f, 3.0f, -5.0f);
    camera->pitch = 10.0f;
    camera->fov_x_degrees = 90.0f;
    camera->aspect_ratio = (real32) display_output->width / display_output->height;
    camera->near = 0.1f;
    camera->far = 1000.0f;
    camera->initial_basis = { z_axis, x_axis, y_axis };
}

void init_game(Game_State *game_state,
               Sound_Output *sound_output, uint32 num_samples) {
#if 0
    char *buf = (char *) pool_push(&memory.string64_pool);
    string_format(buf, 64, "%s", "hello, world!");
    pool_remove(&memory.string64_pool, buf);
    char *buf2 = (char *) pool_push(&memory.string64_pool);
    string_format(buf2, 64, "%s", "wassup");
    char *buf3 = (char *) pool_push(&memory.string64_pool);
    string_format(buf3, 64, "%s", "a profound silence has entered the chat");
    pool_remove(&memory.string64_pool, buf3);
#endif

    Editor_State *editor_state = &game_state->editor_state;

    Arena_Allocator *game_data_arena = &memory.game_data;
    File_Data music_file_data = platform_open_and_read_file((Allocator *) game_data_arena,
                                                            "../drive my car.wav");
    Wav_Data *wav_data = (Wav_Data *) music_file_data.contents;

    Audio_Source *music = &game_state->music;
    *music = make_audio_source(wav_data->subchunk_2_size / (wav_data->bits_per_sample / 8 * 2),
                               0, 1.0f, true, (int16 *) &wav_data->data);

    Camera *camera = &game_state->render_state.camera;
    Display_Output *display_output = &game_state->render_state.display_output;

    // string pool allocators
    Allocator *string64_allocator = (Allocator *) &memory.string64_pool;
    Allocator *filename_allocator = (Allocator *) &memory.filename_pool;
    
    // init tables
    game_state->font_table = make_hash_table<String, Font>((Allocator *) &memory.hash_table_stack,
                                                           HASH_TABLE_SIZE,
                                                           &string_equals);
    game_state->font_file_table = make_hash_table<String, File_Data>((Allocator *) &memory.hash_table_stack,
                                                                     HASH_TABLE_SIZE,
                                                                     &string_equals);
    game_state->mesh_table = make_hash_table<int32, Mesh>((Allocator *) &memory.hash_table_stack,
                                                                  HASH_TABLE_SIZE,
                                                                  &int32_equals);
    game_state->material_table = make_hash_table<int32, Material>((Allocator *) &memory.hash_table_stack,
                                                                   HASH_TABLE_SIZE,
                                                                   &int32_equals);
    game_state->texture_table = make_hash_table<int32, Texture>((Allocator *) &memory.hash_table_stack,
                                                                 HASH_TABLE_SIZE,
                                                                 &int32_equals);

    init_camera(camera, display_output);

    // add meshes
    Allocator *mesh_name_allocator = (Allocator *) &memory.string_arena;
    Mesh mesh;
    mesh = read_and_load_mesh((Allocator *) &memory.mesh_arena,
                              make_string_buffer(filename_allocator, "blender/cube.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "cube", MESH_NAME_MAX_SIZE));
    int32 cube_mesh_id = add_mesh(game_state, mesh);

    mesh = read_and_load_mesh((Allocator *) &memory.mesh_arena,
                              make_string_buffer(filename_allocator, "blender/suzanne2.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "suzanne", MESH_NAME_MAX_SIZE));
    int32 suzanne_mesh_id = add_mesh(game_state, mesh);

    mesh = read_and_load_mesh((Allocator *) &memory.mesh_arena,
                              make_string_buffer(filename_allocator, "blender/gizmo_arrow.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "gizmo_arrow", MESH_NAME_MAX_SIZE));
    int32 gizmo_arrow_mesh_id = add_mesh(game_state, mesh);

    mesh = read_and_load_mesh((Allocator *) &memory.mesh_arena,
                              make_string_buffer(filename_allocator, "blender/gizmo_ring.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "gizmo_ring", MESH_NAME_MAX_SIZE));
    int32 gizmo_ring_mesh_id = add_mesh(game_state, mesh);

    mesh = read_and_load_mesh((Allocator *) &memory.mesh_arena,
                              make_string_buffer(filename_allocator, "blender/gizmo_sphere.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "gizmo_sphere", MESH_NAME_MAX_SIZE));
    int32 gizmo_sphere_mesh_id = add_mesh(game_state, mesh);

    mesh = read_and_load_mesh((Allocator *) &memory.mesh_arena, 
                              make_string_buffer(filename_allocator, "blender/sphere.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "sphere", MESH_NAME_MAX_SIZE));
    int32 sphere_mesh_id = add_mesh(game_state, mesh);

    // init fonts
    Font font;
    font = load_font(game_state, "c:/windows/fonts/times.ttf", "times32", 32.0f, 512, 512);
    add_font(game_state, font);
    font = load_font(game_state, "c:/windows/fonts/times.ttf", "times24", 24.0f, 512, 512);
    add_font(game_state, font);
    font = load_font(game_state, "c:/windows/fonts/courbd.ttf", "courier24b", 24.0f, 512, 512);
    add_font(game_state, font);
    font = load_font(game_state, "c:/windows/fonts/cour.ttf", "courier18", 18.0f, 512, 512);
    add_font(game_state, font);
    font = load_font(game_state, "c:/windows/fonts/courbd.ttf", "courier18b", 18.0f, 512, 512);
    add_font(game_state, font);
    font = load_font(game_state, "c:/windows/fonts/lucon.ttf", "lucidaconsole18", 18.0f, 512, 512);
    add_font(game_state, font);

    // init textures
    Texture texture;
    texture = make_texture(game_state,
                           make_string_buffer(string64_allocator, "debug", 64),
                           make_string_buffer(filename_allocator, "src/textures/debug_texture.png", MAX_PATH));
    int32 debug_texture_id = add_texture(game_state, texture);
    texture = make_texture(game_state,
                           make_string_buffer(string64_allocator, "white", 64),
                           make_string_buffer(filename_allocator, "src/textures/white.bmp", MAX_PATH));
    int32 white_texture_id = add_texture(game_state, texture);

    // init editor_state
    editor_state->gizmo.arrow_mesh_id = gizmo_arrow_mesh_id;
    editor_state->gizmo.ring_mesh_id = gizmo_ring_mesh_id;
    editor_state->gizmo.sphere_mesh_id = gizmo_sphere_mesh_id;
    editor_state->selected_entity_index = -1;
    editor_state->show_wireframe = true;
    editor_state->open_window_flags = 0;
    editor_state->level_name = make_string_buffer((Allocator *) &memory.string64_pool, 64);
    
    // init ui state
    UI_Manager *ui_manager = &game_state->ui_manager;
    UI_Push_Buffer ui_push_buffer = {};
    ui_push_buffer.size = MEGABYTES(1);
    ui_push_buffer.base = allocate((Allocator *) game_data_arena, ui_push_buffer.size);
    ui_push_buffer.used = 0;
    ui_manager->push_buffer = ui_push_buffer;
    ui_manager->current_layer = 0;
    ui_manager->state_table = make_hash_table<UI_id, UI_State_Variant>((Allocator *) &memory.hash_table_stack,
                                                                       HASH_TABLE_SIZE, &ui_id_equals);

    // add materials
    Material shiny_monkey = make_material(make_string_buffer(string64_allocator,
                                                             "shiny_monkey", MATERIAL_STRING_MAX_SIZE),
                                          debug_texture_id,
                                          100.0f, make_vec4(0.6f, 0.6f, 0.6f, 1.0f), true);
    int32 shiny_monkey_material_id = add_material(game_state, shiny_monkey);

    Material plane_material = make_material(make_string_buffer(string64_allocator,
                                                               "diffuse_plane", MATERIAL_STRING_MAX_SIZE),
                                            -1, 1.0f, make_vec4(0.9f, 0.9f, 0.9f, 1.0f), true);
    int32 plane_material_id = add_material(game_state, plane_material);

    Material arrow_material = make_material(make_string_buffer(string64_allocator,
                                                               "arrow_material", MATERIAL_STRING_MAX_SIZE),
                                            -1, 100.0f, make_vec4(1.0f, 0.0f, 0.0f, 1.0f), true);
    int32 arrow_material_id = add_material(game_state, arrow_material);

    Material white_light_material = make_material(make_string_buffer(string64_allocator,
                                                                     "white_light", MATERIAL_STRING_MAX_SIZE),
                                                  -1, 0.0f, make_vec4(1.0f, 1.0f, 1.0f, 1.0f), true);
    int32 white_light_material_id = add_material(game_state, white_light_material);

    Material blue_light_material = make_material(make_string_buffer(string64_allocator,
                                                                    "blue_light", MATERIAL_STRING_MAX_SIZE),
                                                 -1, 0.0f, make_vec4(0.0f, 0.0f, 1.0f, 1.0f), true);
    int32 blue_light_material_id = add_material(game_state, blue_light_material);

    Material diffuse_sphere_material = make_material(make_string_buffer(string64_allocator,
                                                                        "diffuse_sphere", MATERIAL_STRING_MAX_SIZE),
                                                     -1, 5.0f, rgb_to_vec4(176, 176, 176), true);
    int32 diffuse_sphere_material_id = add_material(game_state, diffuse_sphere_material);

    // add entities
    Transform transform = {};
    Normal_Entity entity;

    transform.scale = make_vec3(1.0f, 1.0f, 1.0f);
    transform.position = make_vec3(2.0f, 1.0f, 0.0f);
    transform.rotation = make_quaternion(45.0f, y_axis);
    entity = make_entity(game_state, suzanne_mesh_id, shiny_monkey_material_id, transform);
    add_entity(game_state, entity);

    transform = {};
    transform.scale = make_vec3(5.0f, 0.1f, 5.0f);
    transform.position = make_vec3(0.0f, 0.0f, 0.0f);
    transform.rotation = make_quaternion();
    entity = make_entity(game_state, cube_mesh_id, plane_material_id, transform);
    add_entity(game_state, entity);

    transform = {};
    transform.scale = make_vec3(1.0f, 1.0f, 1.0f);
    transform.position = make_vec3(0.0f, 1.0f, 0.0f);
    transform.rotation = make_quaternion(45.0f, y_axis);
    entity = make_entity(game_state, gizmo_arrow_mesh_id, arrow_material_id, transform);
    add_entity(game_state, entity);

    transform = {};
    transform.scale = make_vec3(0.5f, 0.5f, 0.5f);
    transform.position = make_vec3(-1.5f, 1.5f, -1.0f);
    transform.rotation = make_quaternion();
    entity = make_entity(game_state, sphere_mesh_id, diffuse_sphere_material_id, transform);
    add_entity(game_state, entity);

    Vec3 light_color;
    Point_Light_Entity point_light_entity;

    transform = {};
    transform.scale = make_vec3(0.1f, 0.1f, 0.1f);
    transform.position = make_vec3(0.8f, 1.8f, -2.3f);
    transform.rotation = make_quaternion();
    light_color = make_vec3(0.8f, 0.8f, 0.8f);
    point_light_entity = make_point_light_entity(game_state, cube_mesh_id, white_light_material_id,
                                                 light_color,
                                                 0.0f, 3.0f,
                                                 transform);
    add_point_light_entity(game_state, point_light_entity);

    transform = {};
    transform.scale = make_vec3(0.1f, 0.1f, 0.1f);
    transform.position = make_vec3(-1.0f, 1.5f, 0.0f);
    transform.rotation = make_quaternion();
    light_color = make_vec3(0.0f, 0.0f, 1.0f);
    point_light_entity = make_point_light_entity(game_state, cube_mesh_id, blue_light_material_id,
                                                 light_color,
                                                 0.0f, 5.0f,
                                                 transform);
    add_point_light_entity(game_state, point_light_entity);

    game_state->is_initted = true;
}

inline bool32 was_clicked(Controller_Button_State button_state) {
    return (!button_state.is_down && button_state.was_down);
}

inline bool32 being_held(Controller_Button_State button_state) {
    return (button_state.is_down && button_state.was_down);
}

inline bool32 just_pressed(Controller_Button_State button_state) {
    return (button_state.is_down && !button_state.was_down);
}

inline bool32 just_lifted(Controller_Button_State button_state) {
    return (!button_state.is_down && button_state.was_down);
}

Mat4 get_view_matrix(Camera camera) {
    Basis basis = camera.current_basis;
    return get_view_matrix(camera.position, basis.forward, basis.right, basis.up);
}

Vec3 cursor_pos_to_world_space(Vec2 cursor_pos, Render_State *render_state) {
    Display_Output display_output = render_state->display_output;
    
    Mat4 cpv_matrix_inverse = inverse(render_state->cpv_matrix);

    Vec4 clip_space_coordinates = { 2.0f * (cursor_pos.x / display_output.width) - 1.0f,
                                    -2.0f * (cursor_pos.y / display_output.height) + 1.0f,
                                    -1.0f, 1.0f };

    Vec4 cursor_world_space_homogeneous = cpv_matrix_inverse * clip_space_coordinates;
    Vec3 cursor_world_space = homogeneous_divide(cursor_world_space_homogeneous);

    return cursor_world_space;
}

void update_camera(Camera *camera, Display_Output display_output, Controller_State *controller_state,
                   bool32 use_freecam, bool32 has_focus, bool32 should_move) {
    Basis initial_basis = camera->initial_basis;

    if (use_freecam && has_focus) {
        real32 delta_x = controller_state->current_mouse.x - controller_state->last_mouse.x;
        real32 delta_y = controller_state->current_mouse.y - controller_state->last_mouse.y;

        real32 heading_delta = 0.2f * delta_x;
        real32 pitch_delta = 0.2f * delta_y;

        int32 heading_rotations = (int32) floorf((camera->heading + heading_delta) / 360.0f);
        int32 pitch_rotations = (int32) floorf((camera->pitch + pitch_delta) / 360.0f);
        camera->heading = (camera->heading + heading_delta) - heading_rotations*360.0f;
        camera->pitch = (camera->pitch + pitch_delta) - pitch_rotations*360.0f;
    }
    
    
    Mat4 model_matrix = make_rotate_matrix(camera->roll, camera->pitch, camera->heading);
    Vec3 transformed_forward = truncate_v4_to_v3(model_matrix * make_vec4(initial_basis.forward, 1.0f));
    Vec3 transformed_right = truncate_v4_to_v3(model_matrix * make_vec4(initial_basis.right, 1.0f));
    Vec3 transformed_up = cross(transformed_forward, transformed_right);
    // we calculate a new right vector to correct for any error to ensure that our vectors form an
    // orthonormal basis
    Vec3 corrected_right = cross(transformed_up, transformed_forward);

    Vec3 forward = normalize(transformed_forward);
    Vec3 right = normalize(corrected_right);
    Vec3 up = normalize(transformed_up);

    if (should_move) {
        Vec3 movement_delta = make_vec3();
        if (controller_state->key_w.is_down) {
            movement_delta += forward;
        }
        if (controller_state->key_s.is_down) {
            movement_delta -= forward;
        }
        if (controller_state->key_d.is_down) {
            movement_delta += right;
        }
        if (controller_state->key_a.is_down) {
            movement_delta -= right;
        }
        // TODO: use delta time
        real32 player_speed = 0.3f;
        movement_delta = normalize(movement_delta) * player_speed;
        camera->position += movement_delta;
    }
    
    Basis current_basis = { forward, right, up };
    camera->current_basis = current_basis;
}

void update_render_state(Render_State *render_state) {
    Camera camera = render_state->camera;
    Mat4 view_matrix = get_view_matrix(camera);
    Mat4 perspective_clip_matrix = make_perspective_clip_matrix(camera.fov_x_degrees, camera.aspect_ratio,
                                                                camera.near, camera.far);
    render_state->cpv_matrix = perspective_clip_matrix * view_matrix;

    Display_Output display_output = render_state->display_output;
    Mat4 ortho_clip_matrix = make_ortho_clip_matrix((real32) display_output.width,
                                                    (real32) display_output.height,
                                                    0.0f, 100.0f);
    render_state->ortho_clip_matrix = ortho_clip_matrix;
}

Entity *get_selected_entity(Game_State *game_state) {
    Editor_State *editor_state = &game_state->editor_state;

    Entity *entity = NULL;
    int32 index = editor_state->selected_entity_index;

    switch (editor_state->selected_entity_type) {
        case ENTITY_NORMAL:
        {
            entity = (Entity *) &game_state->normal_entities[index];
        } break;
        case ENTITY_POINT_LIGHT:
        {
            entity = (Entity *) &game_state->point_lights[index];
        } break;
        default: {
            assert (!"Unhandled entity type.");
        } break;
    }

    assert(entity);
    return entity;
}

void update_gizmo(Game_State *game_state) {
    Editor_State *editor_state = &game_state->editor_state;
    if (editor_state->selected_entity_index < 0) return;

    Camera *camera = &game_state->render_state.camera;
    real32 gizmo_scale_factor = distance(editor_state->gizmo.transform.position - camera->position) /
        5.0f;
    editor_state->gizmo.transform.scale = make_vec3(gizmo_scale_factor, gizmo_scale_factor, gizmo_scale_factor);

    Entity *entity = get_selected_entity(game_state);
    editor_state->gizmo.transform.position = entity->transform.position;
    editor_state->gizmo.transform.rotation = entity->transform.rotation;
}

void update(Game_State *game_state,
            Controller_State *controller_state,
            Sound_Output *sound_output, uint32 num_samples) {
    Display_Output *display_output = &game_state->render_state.display_output;
    if (!game_state->is_initted) {
        init_game(game_state, sound_output, num_samples);
        return;
    }

    UI_Manager *ui_manager = &game_state->ui_manager;
    Editor_State *editor_state = &game_state->editor_state;
    Render_State *render_state = &game_state->render_state;

    if (just_pressed(controller_state->key_tab) && !has_focus(ui_manager)) {
        editor_state->use_freecam = !editor_state->use_freecam;
        platform_set_cursor_visible(!editor_state->use_freecam);
    }

    bool32 camera_should_move = editor_state->use_freecam && !has_focus(ui_manager);
    update_camera(&render_state->camera, *display_output, controller_state, editor_state->use_freecam,
                  platform_window_has_focus(), camera_should_move);
    update_render_state(render_state);

    if (editor_state->use_freecam && platform_window_has_focus()) {
        Vec2 center = make_vec2(display_output->width / 2.0f, display_output->height / 2.0f);
        platform_set_cursor_pos(center);
        controller_state->current_mouse = center;
    }

    if (editor_state->use_freecam) {
        disable_input(ui_manager);
    } else {
        enable_input(ui_manager);
    }
    
    
#if 0
    Vec4 cursor_world_space = cursor_pos_to_world_space(game_state->cursor_pos,
                                                        display_output.width, display_output.height,
                                                        &perspective_clip_matrix_inverse,
                                                        &view_matrix_inverse);

    editor_state->last_cursor_ray = editor_state->cursor_ray;
    editor_state->cursor_ray.direction = normalize(truncate_v4_to_v3(cursor_world_space) - camera.position);
#endif
    
    // mesh picking
    Vec3 cursor_world_space = cursor_pos_to_world_space(controller_state->current_mouse,
                                                        &game_state->render_state);
    Ray cursor_ray = { cursor_world_space,
                       normalize(cursor_world_space - render_state->camera.position) };
    
    if (!ui_has_hot(ui_manager) &&
        !ui_has_active(ui_manager) &&
        !editor_state->use_freecam && was_clicked(controller_state->left_mouse)) {
        if (!editor_state->selected_gizmo_handle) {
            Entity entity;
            int32 entity_index;
            bool32 picked = pick_entity(game_state, cursor_ray, &entity, &entity_index);
            
            if (picked) {
                if (selected_entity_changed(editor_state, entity_index, entity.type)) {
                    editor_state->last_selected_entity_type = editor_state->selected_entity_type;
                    editor_state->last_selected_entity_index = editor_state->selected_entity_index;

                    editor_state->selected_entity_type = entity.type;
                    editor_state->selected_entity_index = entity_index;

                    editor_state->gizmo.transform = entity.transform;

                    reset_entity_editors(editor_state);
                }
            } else {
                editor_state->selected_entity_index = -1;
            }
        }
    }

    update_gizmo(game_state);

    if (editor_state->selected_entity_index >= 0 &&
        !ui_has_hot(ui_manager) &&
        !editor_state->selected_gizmo_handle) {

        Vec3 gizmo_initial_hit, gizmo_transform_axis;
        Gizmo_Handle picked_handle = pick_gizmo(game_state, cursor_ray,
                                                &gizmo_initial_hit, &gizmo_transform_axis);
        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
            editor_state->selected_gizmo_handle = picked_handle;
            editor_state->gizmo_initial_hit = gizmo_initial_hit;
            editor_state->gizmo_transform_axis = gizmo_transform_axis;
            editor_state->last_gizmo_transform_point = gizmo_initial_hit;
        } else {
            editor_state->hovered_gizmo_handle = picked_handle;
        }
    }

    if (editor_state->use_freecam ||
        (ui_has_hot(ui_manager) && !controller_state->left_mouse.is_down)) {
        editor_state->hovered_gizmo_handle = GIZMO_HANDLE_NONE;
        editor_state->selected_gizmo_handle = GIZMO_HANDLE_NONE;
    }
    
    if (editor_state->selected_gizmo_handle) {
        disable_input(ui_manager);
        if (controller_state->left_mouse.is_down) {
            Entity *entity = get_selected_entity(game_state);

            if (is_translation(editor_state->selected_gizmo_handle)) {
                Vec3 delta = do_gizmo_translation(&render_state->camera, editor_state, cursor_ray);
                entity->transform.position += delta;
            } else if (is_rotation(editor_state->selected_gizmo_handle)) {
                Quaternion delta = do_gizmo_rotation(&render_state->camera, editor_state, cursor_ray);
                entity->transform.rotation = delta*entity->transform.rotation;
            }

            editor_state->gizmo.transform.position = entity->transform.position;
            editor_state->gizmo.transform.rotation = entity->transform.rotation;
        } else {
            editor_state->selected_gizmo_handle = GIZMO_HANDLE_NONE;
        }
    }

    update_gizmo(game_state);

    draw_editor_ui(game_state, controller_state);
        
    char *buf = (char *) arena_push(&memory.frame_arena, 128);
#if 0
    buf = (char *) arena_push(&memory.frame_arena, 128);
    string_format(buf, 128, "left mouse is down: %d",
                  controller_state->left_mouse.is_down);
    do_text(ui_manager, 0.0f, 516.0f, buf, "times24", "mouse_is_down");
    buf = (char *) arena_push(&memory.frame_arena, 128);
    string_format(buf, 128, "left mouse was down: %d",
                  controller_state->left_mouse.was_down);
    do_text(ui_manager, 0.0f, 500.0f, buf, "times24", "mouse_was_down");
#endif

    buf = (char *) arena_push(&memory.frame_arena, 128);
    string_format(buf, 128, "current_mouse: (%f, %f)",
                  controller_state->current_mouse.x, controller_state->current_mouse.y);
    do_text(ui_manager, 0.0f, 24.0f, buf, "times24", "current_mouse_text");

#if 0
    UI_Image_Button_Style image_button_style = {
        10.0f, 10.0f,
        rgb_to_vec4(33, 62, 69),
        rgb_to_vec4(47, 84, 102),
        rgb_to_vec4(19, 37, 46)
    };
    do_image_button(ui_manager, controller_state, 0, 0, 200.0f, 200.0f,
                    image_button_style, "debug", "debug_image_button");
#endif

#if 0
    UI_Box_Style test_box_style = { make_vec4(1.0f, 0.0f, 0.0f, 1.0f) };
    do_box(ui_manager, controller_state,
           0.0f, 0.0f,
           100.0f, 24.0f,
           test_box_style, "test_box");
#endif

    buf = (char *) arena_push(&memory.frame_arena, 128);
    string_format(buf, 128, "hot: %s\nactive: %s",
                  (char *) ui_manager->hot.string_ptr,
                  (char *) ui_manager->active.string_ptr);
    do_text(ui_manager, 800.0f, 24.0f, buf, "times24", "current_hot");

    buf = (char *) arena_push(&memory.frame_arena, 128);
    string_format(buf, 128, "num ui states: %d",
                  ui_manager->state_table.num_entries);
    do_text(ui_manager, 500.0f, 24.0f, buf, "times24", "num_ui_states");
    buf = (char *) arena_push(&memory.frame_arena, 128);

    buf = (char *) arena_push(&memory.frame_arena, 128);
    string_format(buf, 128, "pool->first: %p",
                  memory.string64_pool.first);
    do_text(ui_manager, 500.0f, 48.0f, buf, "times24", "pool->first");
    buf = (char *) arena_push(&memory.frame_arena, 128);
    
    fill_sound_buffer_with_audio(sound_output, game_state->is_playing_music, &game_state->music, num_samples);

    //game_state->current_char = controller_state->pressed_key;

    clear_hot_if_gone(ui_manager);
    delete_state_if_gone(ui_manager);
    
    assert(ui_manager->current_layer == 0);
}
