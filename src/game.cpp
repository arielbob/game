#include "platform.h"
#include "memory.h"
#include "entity.h"
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

#if 0
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
#endif

int32 add_common_mesh(Game_State *game_state, Mesh mesh) {
    int32 mesh_id = game_state->common_mesh_table.total_added_ever;
    hash_table_add(&game_state->common_mesh_table, mesh_id, mesh);
    return mesh_id;
}

Mesh get_common_mesh(Game_State *game_state, int32 mesh_id) {
    Mesh mesh;
    bool32 mesh_exists = hash_table_find(game_state->common_mesh_table,
                                         mesh_id,
                                         &mesh);
    assert(mesh_exists);
    return mesh;
}

int32 add_primitive_mesh(Game_State *game_state, Mesh mesh) {
    //assert(mesh.is_primitive);
    int32 mesh_id = game_state->primitive_mesh_table.total_added_ever;
    hash_table_add(&game_state->primitive_mesh_table, mesh_id, mesh);
    return mesh_id;
}

#if 0
int32 get_primitive_mesh_id_by_name(Game_State *game_state, String mesh_name) {
    Hash_Table_Iterator<int32, Mesh> iterator = make_hash_table_iterator(game_state->primitive_mesh_table);

    Hash_Table_Entry<int32, Mesh> *entry = get_next_entry_pointer(&iterator);
    while (entry != NULL) {
        if (string_equals(make_string(entry->value.name), mesh_name)) {
            return entry->key;
        }

        entry = get_next_entry_pointer(&iterator);
    }

    return -1;
}
#endif

// TODO: this assumes that mesh names are unique, which isn't currently being enforced.. so ideally we should just
//       not name meshes by the same name.. until we add validation to the text boxes and to the level_add_*
//       procedures.
int32 get_mesh_id_by_name(Game_State *game_state, Level *level, Mesh_Type mesh_type, String mesh_name) {
    int32 num_checked = 0;

    Hash_Table<int32, Mesh> mesh_table;

    if (mesh_type == Mesh_Type::LEVEL) {
        mesh_table = level->mesh_table;
    } else if (mesh_type == Mesh_Type::PRIMITIVE) {
        mesh_table = game_state->primitive_mesh_table;
    } else {
        assert(!"Unhandled mesh type.");
        return -1;
    }

    for (int32 i = 0; (i < mesh_table.max_entries) && (num_checked < mesh_table.num_entries); i++) {
        Hash_Table_Entry<int32, Mesh> entry = mesh_table.entries[i];
        if (entry.is_occupied) {
            if (string_equals(make_string(entry.value.name), mesh_name)) {
                return entry.key;
            }
            num_checked++;
        }
    }

    return -1;
}


Mesh get_mesh(Game_State *game_state, Level *level, Mesh_Type mesh_type, int32 mesh_id) {
    Mesh mesh = {};
    bool32 mesh_exists = false;
    switch (mesh_type) {
        case Mesh_Type::LEVEL: {
            mesh_exists = hash_table_find(level->mesh_table, mesh_id, &mesh);
        } break;
        case Mesh_Type::PRIMITIVE: {
            mesh_exists = hash_table_find(game_state->primitive_mesh_table, mesh_id, &mesh);
        } break;
        default: {
            assert(!"Unhandled mesh type.");
        } break;
    }

    assert(mesh_exists);
    return mesh;
}

Mesh *get_mesh_pointer(Game_State *game_state, Level *level, Mesh_Type mesh_type, int32 mesh_id) {
    Mesh *mesh = NULL;
    bool32 mesh_exists = false;
        switch (mesh_type) {
            case Mesh_Type::LEVEL: {
                mesh_exists = hash_table_find_pointer(level->mesh_table, mesh_id, &mesh);
            } break;
            case Mesh_Type::PRIMITIVE: {
                mesh_exists = hash_table_find_pointer(game_state->primitive_mesh_table, mesh_id, &mesh);
            } break;
            default: {
                assert(!"Unhandled mesh type.");
            } break;
        }

    assert(mesh_exists);
    return mesh;
}

#if 0
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

int32 add_texture(Game_State *game_state, Texture texture) {
    int32 texture_id = game_state->texture_table.total_added_ever;
    hash_table_add(&game_state->texture_table, texture_id, texture);
    return texture_id;
}
#endif

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

    game_state->mode = Game_Mode::EDITING;
    Editor_State *editor_state = &game_state->editor_state;

    real64 current_time = platform_get_wall_clock_time();
    game_state->last_update_time = current_time;
    game_state->last_fps_update_time = current_time;

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
    game_state->common_mesh_table = make_hash_table<int32, Mesh>((Allocator *) &memory.hash_table_stack,
                                                                 HASH_TABLE_SIZE,
                                                                 &int32_equals);
    game_state->primitive_mesh_table = make_hash_table<int32, Mesh>((Allocator *) &memory.hash_table_stack,
                                                                    HASH_TABLE_SIZE,
                                                                    &int32_equals);

    init_camera(camera, display_output);

    // init fonts
    Font font;
    font = load_font(game_state, "c:/windows/fonts/times.ttf", "times32", 32.0f, 512, 512);
    add_font(game_state, font);
    font = load_font(game_state, "c:/windows/fonts/times.ttf", "times24", 24.0f, 512, 512);
    add_font(game_state, font);

    font = load_font(game_state, "c:/windows/fonts/courbd.ttf", "courier24b", 24.0f, 512, 512);
    add_font(game_state, font);

    font = load_font(game_state, "c:/windows/fonts/calibri.ttf", "calibri14", 14.0f, 512, 512);
    add_font(game_state, font);
    font = load_font(game_state, "c:/windows/fonts/calibrib.ttf", "calibri14b", 14.0f, 512, 512);
    add_font(game_state, font);

#if 0
    font = load_font(game_state, "c:/windows/fonts/courbd.ttf", "courier16", 16.0f, 512, 512);
    add_font(game_state, font);
    font = load_font(game_state, "c:/windows/fonts/courbd.ttf", "courier16b", 16.0f, 512, 512);
    add_font(game_state, font);
#endif

    font = load_font(game_state, "c:/windows/fonts/lucon.ttf", "lucidaconsole18", 18.0f, 512, 512);
    add_font(game_state, font);

    // add common meshes
    // NOTE: we just use the string_arena here since we're never going to need to remove these meshes
    Allocator *mesh_name_allocator = (Allocator *) &memory.string_arena;
    Mesh mesh;
    mesh = read_and_load_mesh((Allocator *) &memory.mesh_arena,
                              make_string_buffer(filename_allocator, "blender/gizmo_arrow.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "gizmo_arrow", MESH_NAME_MAX_SIZE));
    int32 gizmo_arrow_mesh_id = add_common_mesh(game_state, mesh);

    mesh = read_and_load_mesh((Allocator *) &memory.mesh_arena,
                              make_string_buffer(filename_allocator, "blender/gizmo_ring.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "gizmo_ring", MESH_NAME_MAX_SIZE));
    int32 gizmo_ring_mesh_id = add_common_mesh(game_state, mesh);

    mesh = read_and_load_mesh((Allocator *) &memory.mesh_arena,
                              make_string_buffer(filename_allocator, "blender/gizmo_sphere.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "gizmo_sphere", MESH_NAME_MAX_SIZE));
    int32 gizmo_sphere_mesh_id = add_common_mesh(game_state, mesh);

    mesh = read_and_load_mesh((Allocator *) &memory.mesh_arena,
                              make_string_buffer(filename_allocator, "blender/cube.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "cube", MESH_NAME_MAX_SIZE));
    add_primitive_mesh(game_state, mesh);

    // init editor_state
    editor_state->gizmo.arrow_mesh_id = gizmo_arrow_mesh_id;
    editor_state->gizmo.ring_mesh_id = gizmo_ring_mesh_id;
    editor_state->gizmo.sphere_mesh_id = gizmo_sphere_mesh_id;
    editor_state->selected_entity_index = -1;
    editor_state->show_wireframe = true;
    editor_state->open_window_flags = 0;
    editor_state->is_new_level = true;
    editor_state->current_level_filename = make_string_buffer(filename_allocator, PLATFORM_MAX_PATH);
    Context::editor_state = editor_state;
    
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
    Context::ui_manager = ui_manager;

    // init level
    Level *current_level = &game_state->current_level;
    current_level->name = make_string_buffer((Allocator *) &memory.level_string64_pool, LEVEL_NAME_MAX_SIZE);
    current_level->mesh_arena = &memory.level_mesh_arena;
    current_level->string64_pool = &memory.level_string64_pool;
    current_level->filename_pool = &memory.level_filename_pool;
    load_default_level(game_state, &game_state->current_level);

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

    Level *level = &game_state->current_level;

    switch (editor_state->selected_entity_type) {
        case ENTITY_NORMAL:
        {
            entity = (Entity *) &level->normal_entities[index];
        } break;
        case ENTITY_POINT_LIGHT:
        {
            entity = (Entity *) &level->point_lights[index];
        } break;
        default: {
            assert (!"Unhandled entity type.");
        } break;
    }

    assert(entity);
    return entity;
}

// TODO: we probably don't always need to update the AABB in some cases; well, idk, there might be uses for AABBs
//       outside of the editor, but that's the only place we're using them right now. although, it is convenient
//       that as long as we use these procedures when transforming entities, the entities will always have an
//       up to date AABB.
void update_entity_position(Game_State *game_state, Entity *entity, Vec3 new_position) {
    entity->transform.position = new_position;
    Mesh *mesh = get_mesh_pointer(game_state, &game_state->current_level, entity->mesh_type, entity->mesh_id);
    entity->transformed_aabb = transform_aabb(mesh->aabb, get_model_matrix(entity->transform));
}

void update_entity_rotation(Game_State *game_state, Entity *entity, Quaternion new_rotation) {
    entity->transform.rotation = new_rotation;
    Mesh *mesh = get_mesh_pointer(game_state, &game_state->current_level, entity->mesh_type, entity->mesh_id);
    entity->transformed_aabb = transform_aabb(mesh->aabb, get_model_matrix(entity->transform));
}

void set_entity_mesh(Game_State *game_state, Level *level, Entity *entity, Mesh_Type mesh_type, int32 mesh_id) {
    Mesh mesh = get_mesh(game_state, level, mesh_type, mesh_id);
    entity->mesh_type = mesh_type;
    entity->mesh_id = mesh_id;
    entity->transformed_aabb = transform_aabb(mesh.aabb, entity->transform);
}

void update(Game_State *game_state,
            Controller_State *controller_state,
            Sound_Output *sound_output, uint32 num_samples) {
    Display_Output *display_output = &game_state->render_state.display_output;
    if (!game_state->is_initted) {
        init_game(game_state, sound_output, num_samples);
        return;
    }

    real64 current_time = platform_get_wall_clock_time();
    real32 dt = (real32) (current_time - game_state->last_update_time);
    game_state->last_update_time = current_time;
    
    game_state->fps_sum += 1.0f / dt;
    game_state->num_fps_samples++;
    if ((current_time - game_state->last_fps_update_time) >= 1.0f) {
        game_state->last_fps_update_time = current_time;
        game_state->last_second_fps = (real32) game_state->fps_sum / game_state->num_fps_samples;
        game_state->num_fps_samples = 0;
        game_state->fps_sum = 0.0f;
    }

    UI_Manager *ui_manager = &game_state->ui_manager;
    Render_State *render_state = &game_state->render_state;

    if (was_clicked(controller_state->key_f5)) {
        if (game_state->mode == Game_Mode::EDITING) {
            game_state->mode = Game_Mode::PLAYING;
        } else {
            game_state->mode = Game_Mode::EDITING;
        }
    }

    if (game_state->mode == Game_Mode::EDITING) {
        update_editor(game_state, controller_state);
        draw_editor_ui(game_state, controller_state);
    }
    
        
    char *buf = (char *) arena_push(&memory.frame_arena, 128);
/*
    buf = (char *) arena_push(&memory.frame_arena, 128);
    string_format(buf, 128, "current_mouse: (%f, %f)",
                  controller_state->current_mouse.x, controller_state->current_mouse.y);
    do_text(ui_manager, 0.0f, 24.0f, buf, "times24", "current_mouse_text");
*/


    char *dt_string = string_format((Allocator *) &memory.frame_arena, 128, "FPS %d / dt %.3f", 
                                    (int32) round(game_state->last_second_fps), dt);
    do_text(ui_manager, 5.0f, 14.0f, dt_string, "calibri14", "dt_string");

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

    clear_hot_if_gone(ui_manager);
    delete_state_if_gone(ui_manager);
    assert(ui_manager->current_layer == 0);
}
