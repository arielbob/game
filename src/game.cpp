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

int32 get_mesh_index(Game_State *game_state, char *mesh_name_to_find) {
    int32 num_meshes = game_state->num_meshes;
    Mesh *meshes = game_state->meshes;

    String mesh_name_to_find_string = make_string(mesh_name_to_find);
    for (int32 i = 0; i < num_meshes; i++) {
        String mesh_name = make_string(meshes[i].name);
        if (string_equals(mesh_name, mesh_name_to_find_string)) {
            return i;
        }
    }

    return -1;
}

int32 get_material_index(Game_State *game_state, char *material_name_to_find) {
    int32 num_materials = game_state->num_materials;
    Material *materials = game_state->materials;

    String material_name_to_find_string = make_string(material_name_to_find);
    for (int32 i = 0; i < num_materials; i++) {
        String material_name = make_string(materials[i].name);
        if (string_equals(material_name, material_name_to_find_string)) {
            return i;
        }
    }

    return -1;
}

Material make_material(String_Buffer material_name,
                       String_Buffer texture_name,
                       real32 specular_exponent,
                       Vec4 color_override, bool32 use_color_override) {
    Material material = { material_name, texture_name,
                          specular_exponent, color_override, use_color_override };
    return material;
}

Normal_Entity make_entity(Game_State *game_state,
                          char *mesh_name,
                          char *material_name,
                          Transform transform) {
    int32 mesh_index = get_mesh_index(game_state, mesh_name);
    assert(mesh_index >= 0);
    int32 material_index = get_material_index(game_state, material_name);
    assert(material_index >= 0);
    Normal_Entity entity = { ENTITY_NORMAL, transform,
                             mesh_index, material_index };
    return entity;
}

Point_Light_Entity make_point_light_entity(Game_State *game_state,
                                           char *mesh_name,
                                           char *material_name,
                                           Vec3 light_color,
                                           real32 d_min, real32 d_max,
                                           Transform transform) {
    int32 mesh_index = get_mesh_index(game_state, mesh_name);
    int32 material_index = get_material_index(game_state, material_name);
    Point_Light_Entity entity = { ENTITY_POINT_LIGHT, transform,
                                  mesh_index, material_index,
                                  light_color, d_min, d_max };
    return entity;
}

void add_mesh(Game_State *game_state, Mesh mesh) {
    assert(game_state->num_meshes < MAX_MESHES);
    game_state->meshes[game_state->num_meshes++] = mesh;
}

void add_entity(Game_State *game_state, Normal_Entity entity) {
    assert(game_state->num_entities < MAX_ENTITIES);
    game_state->entities[game_state->num_entities++] = entity;
}

void add_point_light_entity(Game_State *game_state, Point_Light_Entity entity) {
    assert(game_state->num_point_lights < MAX_POINT_LIGHTS);
    game_state->point_lights[game_state->num_point_lights++] = entity;
}

void add_material(Game_State *game_state, Material material) {
    assert(game_state->num_materials < MAX_MATERIALS);
    game_state->materials[game_state->num_materials++] = material;
}

void add_font(Game_State *game_state, Font font) {
    hash_table_add(&game_state->font_table, make_string(font.name), font);
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

void init_game(Memory *memory, Game_State *game_state,
               Sound_Output *sound_output, uint32 num_samples) {
    Editor_State *editor_state = &game_state->editor_state;

    Arena_Allocator *game_data_arena = &memory->game_data;
    File_Data music_file_data = platform_open_and_read_file((Allocator *) game_data_arena,
                                                            "../drive my car.wav");
    Wav_Data *wav_data = (Wav_Data *) music_file_data.contents;

    Audio_Source *music = &game_state->music;
    *music = make_audio_source(wav_data->subchunk_2_size / (wav_data->bits_per_sample / 8 * 2),
                               0, 1.0f, true, (int16 *) &wav_data->data);

    Camera *camera = &game_state->render_state.camera;
    Display_Output *display_output = &game_state->render_state.display_output;

    game_state->font_table = make_hash_table<Font>((Allocator *) &memory->hash_table_stack);
    game_state->font_file_table = make_hash_table<File_Data>((Allocator *) &memory->hash_table_stack);

    init_camera(camera, display_output);

    // add meshes
    Allocator *mesh_name_allocator = (Allocator *) &memory->string_arena;
    Mesh mesh;
    mesh = read_and_load_mesh(memory, (Allocator *) &memory->mesh_arena, "blender/cube.mesh",
                              make_string_buffer(mesh_name_allocator, "cube", MESH_NAME_MAX_SIZE));
    add_mesh(game_state, mesh);

    mesh = read_and_load_mesh(memory, (Allocator *) &memory->mesh_arena, "blender/suzanne2.mesh",
                              make_string_buffer(mesh_name_allocator, "suzanne", MESH_NAME_MAX_SIZE));
    add_mesh(game_state, mesh);

    mesh = read_and_load_mesh(memory, (Allocator *) &memory->mesh_arena, "blender/gizmo_arrow.mesh",
                              make_string_buffer(mesh_name_allocator, "gizmo_arrow", MESH_NAME_MAX_SIZE));
    add_mesh(game_state, mesh);

    mesh = read_and_load_mesh(memory, (Allocator *) &memory->mesh_arena, "blender/gizmo_ring.mesh",
                              make_string_buffer(mesh_name_allocator, "gizmo_ring", MESH_NAME_MAX_SIZE));
    add_mesh(game_state, mesh);

    mesh = read_and_load_mesh(memory, (Allocator *) &memory->mesh_arena, "blender/gizmo_sphere.mesh",
                              make_string_buffer(mesh_name_allocator, "gizmo_sphere", MESH_NAME_MAX_SIZE));
    add_mesh(game_state, mesh);

    mesh = read_and_load_mesh(memory, (Allocator *) &memory->mesh_arena, "blender/sphere.mesh",
                              make_string_buffer(mesh_name_allocator, "sphere", MESH_NAME_MAX_SIZE));
    add_mesh(game_state, mesh);

    // init fonts
    Font font;
    font = load_font(memory, game_state, "c:/windows/fonts/times.ttf", "times32", 32.0f, 512, 512);
    add_font(game_state, font);
    font = load_font(memory, game_state, "c:/windows/fonts/times.ttf", "times24", 24.0f, 512, 512);
    add_font(game_state, font);
    font = load_font(memory, game_state, "c:/windows/fonts/courbd.ttf", "courier24b", 24.0f, 512, 512);
    add_font(game_state, font);
    font = load_font(memory, game_state, "c:/windows/fonts/cour.ttf", "courier18", 18.0f, 512, 512);
    add_font(game_state, font);
    font = load_font(memory, game_state, "c:/windows/fonts/courbd.ttf", "courier18b", 18.0f, 512, 512);
    add_font(game_state, font);
    font = load_font(memory, game_state, "c:/windows/fonts/lucon.ttf", "lucidaconsole18", 18.0f, 512, 512);
    add_font(game_state, font);

    // init editor_state
    editor_state->gizmo.arrow_mesh_name = "gizmo_arrow";
    editor_state->gizmo.ring_mesh_name = "gizmo_ring";
    editor_state->gizmo.sphere_mesh_name = "gizmo_sphere";
    editor_state->selected_entity_index = -1;

    // init ui state
    UI_Manager *ui_manager = &game_state->ui_manager;
    UI_Push_Buffer ui_push_buffer = {};
    ui_push_buffer.size = KILOBYTES(8);
    ui_push_buffer.base = allocate((Allocator *) game_data_arena, ui_push_buffer.size);
    ui_push_buffer.used = 0;
    ui_manager->push_buffer = ui_push_buffer;

    // add materials
    Allocator *material_string_allocator = (Allocator *) &memory->string_arena;
    Material shiny_monkey = make_material(make_string_buffer(material_string_allocator,
                                                             "shiny_monkey", MATERIAL_NAME_MAX_SIZE),
                                          make_string_buffer(material_string_allocator,
                                                             "debug", MATERIAL_NAME_MAX_SIZE),
                                          100.0f, make_vec4(0.6f, 0.6f, 0.6f, 1.0f), true);
    add_material(game_state, shiny_monkey);

    Material plane_material = make_material(make_string_buffer(material_string_allocator,
                                                               "diffuse_plane", MATERIAL_NAME_MAX_SIZE),
                                            make_string_buffer(material_string_allocator, MATERIAL_NAME_MAX_SIZE),
                                            1.0f, make_vec4(0.9f, 0.9f, 0.9f, 1.0f), true);
    add_material(game_state, plane_material);

    Material arrow_material = make_material(make_string_buffer(material_string_allocator,
                                                               "arrow_material", MATERIAL_NAME_MAX_SIZE),
                                            make_string_buffer(material_string_allocator, MATERIAL_NAME_MAX_SIZE),
                                            100.0f, make_vec4(1.0f, 0.0f, 0.0f, 1.0f), true);
    add_material(game_state, arrow_material);

    Material white_light_material = make_material(make_string_buffer(material_string_allocator,
                                                                     "white_light", MATERIAL_NAME_MAX_SIZE),
                                                  make_string_buffer(material_string_allocator,
                                                                     MATERIAL_NAME_MAX_SIZE),
                                                  0.0f, make_vec4(1.0f, 1.0f, 1.0f, 1.0f), true);
    add_material(game_state, white_light_material);
    Material blue_light_material = make_material(make_string_buffer(material_string_allocator,
                                                                    "blue_light", MATERIAL_NAME_MAX_SIZE),
                                                 make_string_buffer(material_string_allocator,
                                                                    MATERIAL_NAME_MAX_SIZE),
                                                 0.0f, make_vec4(0.0f, 0.0f, 1.0f, 1.0f), true);
    add_material(game_state, blue_light_material);
    Material diffuse_sphere_material = make_material(make_string_buffer(material_string_allocator,
                                                                        "diffuse_sphere", MATERIAL_NAME_MAX_SIZE),
                                                     make_string_buffer(material_string_allocator,
                                                                        MATERIAL_NAME_MAX_SIZE),
                                                     5.0f, rgb_to_vec4(176, 176, 176), true);
    add_material(game_state, diffuse_sphere_material);

    // add entities
    Transform transform = {};
    Normal_Entity entity;

    transform.scale = make_vec3(1.0f, 1.0f, 1.0f);
    transform.position = make_vec3(2.0f, 1.0f, 0.0f);
    transform.rotation = make_quaternion(45.0f, y_axis);
    entity = make_entity(game_state, "suzanne", "shiny_monkey", transform);
    add_entity(game_state, entity);

    transform = {};
    transform.scale = make_vec3(5.0f, 0.1f, 5.0f);
    transform.position = make_vec3(0.0f, 0.0f, 0.0f);
    transform.rotation = make_quaternion();
    entity = make_entity(game_state, "cube", "diffuse_plane", transform);
    add_entity(game_state, entity);

    transform = {};
    transform.scale = make_vec3(1.0f, 1.0f, 1.0f);
    transform.position = make_vec3(0.0f, 1.0f, 0.0f);
    transform.rotation = make_quaternion(45.0f, y_axis);
    entity = make_entity(game_state, "gizmo_arrow", "arrow_material", transform);
    add_entity(game_state, entity);

    transform = {};
    transform.scale = make_vec3(0.5f, 0.5f, 0.5f);
    transform.position = make_vec3(-1.5f, 1.5f, -1.0f);
    transform.rotation = make_quaternion();
    entity = make_entity(game_state, "sphere", "diffuse_sphere", transform);
    add_entity(game_state, entity);

    Vec3 light_color;
    Point_Light_Entity point_light_entity;

    transform = {};
    transform.scale = make_vec3(0.1f, 0.1f, 0.1f);
    transform.position = make_vec3(0.8f, 1.8f, -2.3f);
    transform.rotation = make_quaternion();
    light_color = make_vec3(0.8f, 0.8f, 0.8f);
    point_light_entity = make_point_light_entity(game_state, "cube", "white_light",
                                                 light_color,
                                                 0.0f, 3.0f,
                                                 transform);
    add_point_light_entity(game_state, point_light_entity);

    transform = {};
    transform.scale = make_vec3(0.1f, 0.1f, 0.1f);
    transform.position = make_vec3(-1.0f, 1.5f, 0.0f);
    transform.rotation = make_quaternion();
    light_color = make_vec3(0.0f, 0.0f, 1.0f);
    point_light_entity = make_point_light_entity(game_state, "cube", "blue_light",
                                                 light_color,
                                                 0.0f, 5.0f,
                                                 transform);
    add_point_light_entity(game_state, point_light_entity);

    game_state->is_initted = true;
}

inline bool32 was_clicked(Controller_Button_State button_state) {
    return (!button_state.is_down && button_state.was_down);
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
            entity = (Entity *) &game_state->entities[index];
        } break;
        case ENTITY_POINT_LIGHT:
        {
            entity = (Entity *) &game_state->point_lights[index];
        } break;
    }

    assert(entity);
    return entity;
}

void update_gizmo(Game_State *game_state) {
    Editor_State *editor_state = &game_state->editor_state;
    Camera *camera = &game_state->render_state.camera;
    real32 gizmo_scale_factor = distance(editor_state->gizmo.transform.position - camera->position) /
        5.0f;
    editor_state->gizmo.transform.scale = make_vec3(gizmo_scale_factor, gizmo_scale_factor, gizmo_scale_factor);

    Entity *entity = get_selected_entity(game_state);
    editor_state->gizmo.transform.position = entity->transform.position;
    editor_state->gizmo.transform.rotation = entity->transform.rotation;
}

void update(Memory *memory, Game_State *game_state,
            Controller_State *controller_state,
            Sound_Output *sound_output, uint32 num_samples) {
    Display_Output *display_output = &game_state->render_state.display_output;
    if (!game_state->is_initted) {
        init_game(memory, game_state, sound_output, num_samples);
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

#if 0
    //game_state->left_mouse_is_down = controller_state->left_mouse.is_down;
    bool32 btn1_clicked = do_text_button(ui_manager, controller_state,
                                         20.0f, 50.0f, 100.0f, 30.0f,
                                         "load mesh", "times24", "load_mesh");
    bool32 btn2_clicked = do_text_button(ui_manager, controller_state,
                                         50.0f, 360.0f, 200.0f, 30.0f,
                                         "toggle music", "times24", "toggle_music");

    // TODO: GetOpenFileName blocks, so we should do the open file dialog stuff on a separate thread.
    //       https://docs.microsoft.com/en-us/windows/win32/procthread/processes-and-threads
    if (btn1_clicked) {
        char *filepath = (char *) arena_push(&memory->string_arena, PLATFORM_MAX_PATH);
        char *mesh_name_buffer = (char *) arena_push(&memory->string_arena, MESH_NAME_MAX_SIZE);

        if (platform_open_file_dialog(filepath, PLATFORM_MAX_PATH)) {
            //Marker m = begin_region(memory);
            //File_Data file_data = platform_open_and_read_file((Allocator *) &memory->global_stack, filepath);
            //end_region(memory, m);

            // TODO: prompt user to enter name for mesh; just using filepath name for now
            Mesh mesh = read_and_load_mesh(memory, (Allocator *) &memory->mesh_arena, filepath,
                                           mesh_name_buffer, MESH_NAME_MAX_SIZE);
            game_state->is_naming_mesh = true;
            game_state->mesh_to_add = mesh;

            // add_mesh(game_state, mesh);

#if 0
            Transform transform = {};
            transform.scale = make_vec3(1.0f, 1.0f, 1.0f);
            Entity entity = make_entity(mesh.name, transform);
            add_entity(game_state, entity);
#endif
        }
    }

    if (game_state->is_naming_mesh) {
        UI_Text_Box_Style style = {
            "times24",
            500.0f, 30.0f,
            5.0f, 5.0f,
        };

        do_text_box(ui_manager, controller_state,
                    251.0f, 360.0f,
                    game_state->mesh_to_add.name, game_state->mesh_to_add.name_size,
                    style, "mesh_name_text_box");
        bool32 submit_clicked = do_text_button(ui_manager, controller_state,
                                               765.0f, 360.0f, 200.0f, 30.0f,
                                               "submit", "times24", "mesh_name_text_box_submit");
        if (submit_clicked) {
            game_state->is_naming_mesh = false;
            add_mesh(game_state, game_state->mesh_to_add);
        }
        // TODO: add a cancel button that cancels this operation and deallocates the mesh strings and mesh data
        // TODO: we may want to think of a better API for strings, since it's kind of painful right now
    }

    if (btn2_clicked) {
        game_state->is_playing_music = !game_state->is_playing_music;
    }
#endif

#if 0
    UI_Text_Box_Style style = {
        "times24",
        500.0f, 30.0f,
        5.0f, 5.0f,
    };

    do_text_box(ui_manager, controller_state,
                251.0f, 360.0f,
                game_state->text_buffer, 256,
                style, "mesh_name_text_box");
#endif
    
    char *toggle_transform_mode_text;
    if (editor_state->transform_mode == TRANSFORM_GLOBAL) {
        toggle_transform_mode_text = "use local transform";
    } else {
        toggle_transform_mode_text = "use global transform";
    }

    UI_Text_Button_Style style = { rgb_to_vec4(33, 62, 69),
                                   rgb_to_vec4(47, 84, 102),
                                   rgb_to_vec4(19, 37, 46),
                                   make_vec4(1.0f, 1.0f, 1.0f, 1.0f) };
    bool32 toggle_global_clicked = do_text_button(ui_manager, controller_state,
                                                  765.0f, 360.0f,
                                                  250.0f, 60.0f,
                                                  style,
                                                  toggle_transform_mode_text, "times24", "toggle_transform");
    if (toggle_global_clicked) {
        if (editor_state->transform_mode == TRANSFORM_GLOBAL) {
            editor_state->transform_mode = TRANSFORM_LOCAL;
        } else {
            editor_state->transform_mode = TRANSFORM_GLOBAL;
        }
    }

    // mesh picking
    Vec3 cursor_world_space = cursor_pos_to_world_space(controller_state->current_mouse,
                                                        &game_state->render_state);
    Ray cursor_ray = { cursor_world_space,
                       normalize(cursor_world_space - render_state->camera.position) };
    
    if (!ui_has_hot(ui_manager) && !editor_state->use_freecam && was_clicked(controller_state->left_mouse)) {
        if (!editor_state->selected_gizmo_handle) {
            Entity entity;
            int32 entity_index;
            bool32 picked = pick_entity(game_state, cursor_ray, &entity, &entity_index);
            
            if (picked) {
                editor_state->selected_entity_type = entity.type;
                editor_state->selected_entity_index = entity_index;
                editor_state->gizmo.transform = entity.transform;
            } else {
                editor_state->selected_entity_index = -1;
            }
        }
    }

    if (editor_state->selected_entity_index >= 0) {
        draw_entity_box(memory, game_state, controller_state, get_selected_entity(game_state));
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
    
    char *buf = (char *) arena_push(&memory->frame_arena, 128);
    string_format(buf, 128, "picked gizmo: %d",
                  editor_state->selected_gizmo_handle);
    do_text(ui_manager, 0.0f, 600.0f, buf, "times24", "picked_gizmo_text");

    buf = (char *) arena_push(&memory->frame_arena, 128);
    string_format(buf, 128, "hovered gizmo: %d",
                  editor_state->hovered_gizmo_handle);
    do_text(ui_manager, 0.0f, 564.0f, buf, "times24", "picked_gizmo_text");

    buf = (char *) arena_push(&memory->frame_arena, 128);
    string_format(buf, 128, "left mouse is down: %d",
                  controller_state->left_mouse.is_down);
    do_text(ui_manager, 0.0f, 516.0f, buf, "times24", "mouse_is_down");
    buf = (char *) arena_push(&memory->frame_arena, 128);
    string_format(buf, 128, "left mouse was down: %d",
                  controller_state->left_mouse.was_down);
    do_text(ui_manager, 0.0f, 500.0f, buf, "times24", "mouse_was_down");

    buf = (char *) arena_push(&memory->frame_arena, 128);
    string_format(buf, 128, "selected entity type: %d\nselected entity index: %d",
                  editor_state->selected_entity_type, editor_state->selected_entity_index);
    do_text(ui_manager, 0.0f, 370.0f, buf, "times24", "selected_entity_index_text");

    buf = (char *) arena_push(&memory->frame_arena, 128);
    string_format(buf, 128, "current_mouse: (%f, %f)",
                  controller_state->current_mouse.x, controller_state->current_mouse.y);
    do_text(ui_manager, 0.0f, 24.0f, buf, "times24", "current_mouse_text");

    buf = (char *) arena_push(&memory->frame_arena, 128);
    string_format(buf, 128, "camera heading: %f\ncamera pitch: %f",
                  render_state->camera.heading,
                  render_state->camera.pitch);
    do_text(ui_manager, 500.0f, 24.0f, buf, "times24", "camera_info");

    buf = (char *) arena_push(&memory->frame_arena, 128);
    string_format(buf, 128, "hot: %s",
                  (char *) ui_manager->hot.string_ptr);
    do_text(ui_manager, 800.0f, 24.0f, buf, "times24", "current_hot");

    UI_Image_Button_Style image_button_style = {
        10.0f, 10.0f,
        rgb_to_vec4(33, 62, 69),
        rgb_to_vec4(47, 84, 102),
        rgb_to_vec4(19, 37, 46)
    };
    do_image_button(ui_manager, controller_state, 0, 0, 200.0f, 200.0f,
                    image_button_style, "debug", "debug_image_button");

    fill_sound_buffer_with_audio(sound_output, game_state->is_playing_music, &game_state->music, num_samples);

    //game_state->current_char = controller_state->pressed_key;
}
