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

bool32 material_name_exists(Level *level, String name) {
    FOR_VALUE_POINTERS(int32, Material, level->material_table) {
        Material *material = value;
        if (string_equals(make_string(value->name), name)) {
            return true;
        }
    }

    return false;
}

bool32 texture_name_exists(Level *level, String name) {
    FOR_VALUE_POINTERS(int32, Texture, level->texture_table) {
        if (string_equals(make_string(value->name), name)) {
            return true;
        }
    }

    return false;
}

bool32 mesh_name_exists(Game_State *game_state, Level *level, String name) {
    {
        FOR_VALUE_POINTERS(int32, Mesh, game_state->primitive_mesh_table) {
            if (string_equals(make_string(value->name), name)) {
                return true;
            }
        }
    }

    {
        FOR_VALUE_POINTERS(int32, Mesh, level->mesh_table) {
            if (string_equals(make_string(value->name), name)) {
                return true;
            }
        }
    }

    return false;
}

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


bool32 mesh_exists(Game_State *game_state, Level *level, Mesh_Type mesh_type, int32 mesh_id) {
    bool32 mesh_exists = false;
    switch (mesh_type) {
        case Mesh_Type::LEVEL: {
            mesh_exists = hash_table_exists(level->mesh_table, mesh_id);
        } break;
        case Mesh_Type::PRIMITIVE: {
            mesh_exists = hash_table_exists(game_state->primitive_mesh_table, mesh_id);
        } break;
        default: {
            assert(!"Unhandled mesh type.");
        } break;
    }
    return mesh_exists;
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
    Heap_Allocator *heap = &memory.level_mesh_heap;
    void *a = heap_allocate(heap, 64);
    void *b = heap_allocate(heap, 24);
    void *c = heap_allocate(heap, 20);

    heap_deallocate(heap, b);
    heap_deallocate(heap, c);
    heap_deallocate(heap, a);
    heap_allocate(heap, 2);
#endif

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

    game_state->player.height = 1.6f;

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

    // init message manager
    game_state->message_manager = {};
    game_state->message_manager.message_time_limit = MESSAGE_TIME_LIMIT;
    Context::message_manager = &game_state->message_manager;

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

    font = load_font(game_state, "c:/windows/fonts/calibrib.ttf", "calibri24b", 24.0f, 512, 512);
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
    mesh = read_and_load_mesh((Allocator *) &memory.common_mesh_arena,
                              make_string_buffer(filename_allocator, "blender/gizmo_arrow.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "gizmo_arrow", MESH_NAME_MAX_SIZE));
    int32 gizmo_arrow_mesh_id = add_common_mesh(game_state, mesh);

    mesh = read_and_load_mesh((Allocator *) &memory.common_mesh_arena,
                              make_string_buffer(filename_allocator, "blender/gizmo_ring.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "gizmo_ring", MESH_NAME_MAX_SIZE));
    int32 gizmo_ring_mesh_id = add_common_mesh(game_state, mesh);

    mesh = read_and_load_mesh((Allocator *) &memory.common_mesh_arena,
                              make_string_buffer(filename_allocator, "blender/gizmo_sphere.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "gizmo_sphere", MESH_NAME_MAX_SIZE));
    int32 gizmo_sphere_mesh_id = add_common_mesh(game_state, mesh);

    mesh = read_and_load_mesh((Allocator *) &memory.common_mesh_arena,
                              make_string_buffer(filename_allocator, "blender/gizmo_cube.mesh",
                                                 PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "gizmo_cube", MESH_NAME_MAX_SIZE));
    int32 gizmo_cube_mesh_id = add_common_mesh(game_state, mesh);

    mesh = read_and_load_mesh((Allocator *) &memory.common_mesh_arena,
                              make_string_buffer(filename_allocator, "blender/cube.mesh", PLATFORM_MAX_PATH),
                              make_string_buffer(mesh_name_allocator, "cube", MESH_NAME_MAX_SIZE));
    add_primitive_mesh(game_state, mesh);

    Vec3 origin = make_vec3(0.0f, 10.0f, 0.0f);
    Vec3 direction = make_vec3(0.0f, -1.0f, 0.0f);
    Ray ray = make_ray(origin, direction);
    Transform transform = make_transform();
    transform.scale = make_vec3(1.0f, 0.5f, 1.0f);

    // init editor_state
    editor_state->gizmo.arrow_mesh_id = gizmo_arrow_mesh_id;
    editor_state->gizmo.ring_mesh_id = gizmo_ring_mesh_id;
    editor_state->gizmo.sphere_mesh_id = gizmo_sphere_mesh_id;
    editor_state->gizmo.cube_mesh_id = gizmo_cube_mesh_id;
    editor_state->selected_entity_id = -1;
    editor_state->show_wireframe = true;
    editor_state->open_window_flags = 0;
    editor_state->is_new_level = true;
    editor_state->current_level_filename = make_string_buffer(filename_allocator, PLATFORM_MAX_PATH);
    Context::editor_state = editor_state;
    
    Editor_History editor_history = {};
    editor_history.allocator_pointer = (Allocator *) &memory.editor_history_heap;
    editor_state->history = editor_history;
    
    // init ui state
    UI_Manager *ui_manager = &game_state->ui_manager;
    UI_Push_Buffer ui_push_buffer = {};
    ui_push_buffer.size = MEGABYTES(1);
    ui_push_buffer.base = allocate((Allocator *) game_data_arena, ui_push_buffer.size);
    ui_push_buffer.used = 0;
    ui_manager->push_buffer = ui_push_buffer;
    ui_manager->current_layer = 0;
    // ui_manager->state_table = make_hash_table<UI_id, UI_State_Variant>((Allocator *) &memory.hash_table_stack,
    //                                                                    HASH_TABLE_SIZE, &ui_id_equals);
    ui_manager->heap_pointer = &memory.ui_state_heap;
    ui_manager->state_table = make_hash_table<UI_id, UI_Element_State *>((Allocator *) &memory.hash_table_stack,
                                                                         HASH_TABLE_SIZE, &ui_id_equals);
    Context::ui_manager = ui_manager;

    // init level
    Level *current_level = &game_state->current_level;
    current_level->name = make_string_buffer((Allocator *) &memory.level_string64_pool, LEVEL_NAME_MAX_SIZE);
    current_level->mesh_heap_pointer = &memory.level_mesh_heap;
    current_level->arena_pointer = &memory.level_arena;
    current_level->string64_pool_pointer = &memory.level_string64_pool;
    current_level->filename_pool_pointer = &memory.level_filename_pool;
    load_default_level(game_state, &game_state->current_level);

    game_state->is_initted = true;
}

void reset_debug_state(Debug_State *debug_state) {
    debug_state->num_debug_lines = 0;
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

int32 ray_intersects_mesh(Ray ray, Mesh mesh, Transform transform, bool32 include_backside,
                          Ray_Intersects_Mesh_Result *result) {
    Mat4 object_to_world = get_model_matrix(transform);
    Mat4 world_to_object = inverse(object_to_world);

    // instead of transforming every vertex, we just transform the world-space ray to object-space by
    // multiplying the origin and the direction of the ray by the inverse of the entity's model matrix.
    // note that we must zero-out the w component of the ray direction when doing this multiplication so that
    // we ignore the translation of the model matrix.
    Vec3 object_space_ray_origin = truncate_v4_to_v3(world_to_object * make_vec4(ray.origin, 1.0f));
    // NOTE: we do NOT normalize here, since this allows you to pass in a line represented as a ray, i.e.
    //       a ray where the origin is the start and the direction is a vector in the direction of the line
    //       that has the length of the line. you can then just check if t is between 0 and 1 to check if
    //       the intersection occured on the line.
    Vec3 object_space_ray_direction = truncate_v4_to_v3(world_to_object *
                                                        make_vec4(ray.direction, 0.0f));
    Ray object_space_ray = { object_space_ray_origin, object_space_ray_direction };

    uint32 *indices = mesh.indices;

    // this might be very slow - this is very cache unfriendly since we're constantly jumping around the
    // vertices array, which in some cases is large.
    // we could cache every single triangle vertex in an array of size num_triangles*3, so our
    // suzanne mesh with 62976 triangles would take up 62976*3*sizeof(real32) = 755712 bytes = 756KB,
    // which isn't terrible, but i'm not sure we even have a need for a fast ray_intersects_mesh()
    // right now, so this way of doing it is fine for now.
    real32 t_min = FLT_MAX;
    bool32 hit = false;
    int32 hit_triangle_index = -1;
    for (int32 i = 0; i < (int32) mesh.num_triangles; i++) {
        Vec3 triangle[3];
        triangle[0] = get_vertex_from_index(&mesh, indices[i * 3]);
        triangle[1] = get_vertex_from_index(&mesh, indices[i * 3 + 1]);
        triangle[2] = get_vertex_from_index(&mesh, indices[i * 3 + 2]);
        real32 t;

        // TODO: we might be able to pass t_min into this test to early-out before we check if a hit point
        //       is within the triangle, but after we've hit the plane
        if (ray_intersects_triangle(object_space_ray, triangle, include_backside, &t)) {
            if (t < t_min) {
                t_min = t;
                hit_triangle_index = i;
            }
            hit = true;
        }
    }

    if (hit) {
        Vec3 triangle[3];
        Vec3 v1 = get_vertex_from_index(&mesh, indices[hit_triangle_index * 3]);
        Vec3 v2 = get_vertex_from_index(&mesh, indices[hit_triangle_index * 3 + 1]);
        Vec3 v3 = get_vertex_from_index(&mesh, indices[hit_triangle_index * 3 + 2]);

        triangle[0] = transform_point(&object_to_world, &v1);
        triangle[1] = transform_point(&object_to_world, &v2);
        triangle[2] = transform_point(&object_to_world, &v3);

        result->t = t_min;
        result->triangle_index = hit_triangle_index;
        result->triangle_normal = get_triangle_normal(triangle);
    }

    return hit;
}

bool32 closest_point_below_on_mesh(Vec3 point, Mesh mesh, Transform transform,
                                   Vec3 *result) {
    // NOTE: we cannot do the same optimization here where we transform the world space point to object
    //       space and do the test in obejct space. this is because get_point_on_triangle_from_xz assumes
    //       that the line is going straight down. we can do the optimization in ray_intersects_mesh
    //       because we have a ray that we can transform. in this procedure, we cannot transform the "ray"
    //       so we must transform the triangles to world space before doing the test.
    // TODO: we can write an optimized version of this that takes in a list of transformed triangles.
    //       we can generate those transformed triangles for the walk mesh, and assuming that the walk
    //       mesh doesn't change, we can just do it once and have the benefits of not having to use the
    //       indices array.
    // TODO: SIMDize the matrix multiplication code
    // TODO: we can probably also cache some things to make get_point_on_triangle_from_xz() faster
    Mat4 object_to_world = get_model_matrix(transform);

    uint32 *indices = mesh.indices;

    Vec3 closest_point_below = make_vec3();
    bool32 hit = false;
    for (int32 i = 0; i < (int32) mesh.num_triangles; i++) {
        Vec3 triangle[3];
        Vec3 v1 = get_vertex_from_index(&mesh, indices[i*3]);
        Vec3 v2 = get_vertex_from_index(&mesh, indices[i*3 + 1]);
        Vec3 v3 = get_vertex_from_index(&mesh, indices[i*3 + 2]);
        triangle[0] = transform_point(&object_to_world, &v1);
        triangle[1] = transform_point(&object_to_world, &v2);
        triangle[2] = transform_point(&object_to_world, &v3);
        Vec3 point_on_triangle;
        if (get_point_on_triangle_from_xz(point.x, point.z, triangle, &point_on_triangle)) {
            if (point_on_triangle.y < point.y) {
                if (!hit) {
                    closest_point_below = point_on_triangle;
                    hit = true;
                } else {
                    if (point_on_triangle.y > closest_point_below.y) {
                        closest_point_below = point_on_triangle;
                    }
                }
            }
        }
    }

    if (hit) *result = closest_point_below;

    return hit;
}

// TODO: this is VERY slow
bool32 capsule_intersects_mesh(Capsule capsule, Mesh mesh, Transform transform) {
    Mat4 object_to_world = get_model_matrix(transform);

    uint32 *indices = mesh.indices;

    for (int32 i = 0; i < (int32) mesh.num_triangles; i++) {
        Vec3 triangle[3];
        get_triangle(&mesh, i, triangle);
        transform_triangle(triangle, &object_to_world);

        Vec3 penetration_normal;
        real32 penetration_depth;
        if (capsule_intersects_triangle(capsule, triangle, &penetration_normal, &penetration_depth)) {
            return true;
        }
    }

    return false;
}

// gets the highest point within the cylinder of some radius around some center from min_y to max_y
bool32 get_walkable_triangle_on_mesh(Vec3 center, real32 radius,
                                     Mesh *mesh, Transform transform,
                                     real32 min_y, real32 max_y,
                                     Get_Walkable_Triangle_On_Mesh_Result *result) {
    Mat4 object_to_world = get_model_matrix(transform);

    uint32 *indices = mesh->indices;

    bool32 found = false;
    Vec3 highest_point_in_range = make_vec3();
    real32 found_point_height = min_y;
    int32 triangle_index = -1;
    Vec3 found_triangle_normal = make_vec3();
    for (int32 i = 0; i < (int32) mesh->num_triangles; i++) {
        Vec3 triangle[3];
        get_triangle(mesh, i, triangle);
        transform_triangle(triangle, &object_to_world);
        Vec3 triangle_normal = get_triangle_normal(triangle);
        if (fabs(triangle_normal.y) < EPSILON) continue;

        // TODO: we could do a check for slope angle here instead of just checking for < 0.0f.
        // NOTE: we check for < 0, since we only want to be standing on triangles if they're pointing up and if
        //       y is negative, then it's pointing downwards and we only want to step on it if the mesh is
        //       double sided.
        if (!mesh->is_double_sided && triangle_normal.y < 0.0f) continue;

        if (circle_intersects_triangle_on_xz_plane(center, radius, triangle, triangle_normal)) {
            Vec3 point_on_triangle_plane = get_point_on_plane_from_xz(center.x, center.z,
                                                                      triangle_normal, triangle[0]);

            real32 point_y = point_on_triangle_plane.y;
            if ((point_y > found_point_height) && (point_y > min_y) && (point_y < max_y)) {
                found_point_height = point_y;
                highest_point_in_range = point_on_triangle_plane;
                triangle_index = i;
                found = true;
                found_triangle_normal = triangle_normal;
            }
        }
    }

    if (found) {
        result->point = highest_point_in_range;
        result->triangle_index = triangle_index;
        result->triangle_normal = found_triangle_normal;
    }

    return found;
}

bool32 closest_vertical_point_on_mesh(Vec3 point, Mesh *mesh, Transform transform,
                                      real32 max_distance,
                                      Closest_Vertical_Point_On_Mesh_Result *result) {
    Mat4 object_to_world = get_model_matrix(transform);

    uint32 *indices = mesh->indices;

    bool32 found = false;
    Vec3 closest_point = make_vec3();
    real32 distance_to_closest_point = FLT_MAX;
    int32 triangle_index = -1;
    for (int32 i = 0; i < (int32) mesh->num_triangles; i++) {
        Vec3 triangle[3];
        get_triangle(mesh, i, triangle);
        transform_triangle(triangle, &object_to_world);

        Vec3 point_on_triangle;
        if (get_point_on_triangle_from_xz(point.x, point.z, triangle, &point_on_triangle)) {
            real32 distance_to_point = distance(point - point_on_triangle);
            if ((distance_to_point < max_distance) && (distance_to_point < distance_to_closest_point)) {
                closest_point = point;
                distance_to_closest_point = distance_to_point;
                found = true;
                triangle_index = i;
            }
        }
    }

    if (found) {
        result->point = closest_point;
        result->distance_to_point = distance_to_closest_point;
        result->triangle_index = triangle_index;

        Vec3 triangle[3];
        get_triangle(mesh, triangle_index, triangle);
        transform_triangle(triangle, &object_to_world);
        Vec3 normal = get_triangle_normal(triangle);

        result->triangle_normal = normal;
    }

    return found;
}

bool32 get_new_walk_state(Game_State *game_state, Walk_State current_walk_state, Vec3 player_position,
                          Walk_State *walk_state_result, Vec3 *grounded_position) {
    Level *level = &game_state->current_level;

    Circle_Collider player_collider = make_circle_collider(player_position, Player_Constants::walk_radius);
    real32 max_lower_offset = Player_Constants::max_lower_ground_offset;
    real32 max_upper_offset = Player_Constants::max_upper_ground_offset;

    Vec3 highest_point = make_vec3(0.0f, player_collider.center.y - max_lower_offset, 0.0f);
    int32 triangle_index = -1;
    Vec3 triangle_normal = make_vec3();
    bool32 found_walkable_point = false;
    Entity_Type ground_entity_type = ENTITY_NONE;
    int32 ground_entity_id = -1;

    FOR_ENTRY_POINTERS(int32, Normal_Entity, level->normal_entity_table) {
        Normal_Entity *entity = &entry->value;
        if (entity->is_walkable) {
            Mesh *mesh = get_mesh_pointer(game_state, level, entity->mesh_type, entity->mesh_id);
            Get_Walkable_Triangle_On_Mesh_Result result;
            bool32 found_triangle = get_walkable_triangle_on_mesh(player_collider.center, player_collider.radius,
                                                                  mesh,
                                                                  entity->transform,
                                                                  player_collider.center.y - max_lower_offset,
                                                                  player_collider.center.y + max_upper_offset,
                                                                  &result);
            if (found_triangle && (result.point.y > highest_point.y)) {
                highest_point = result.point;
                triangle_index = result.triangle_index;
                triangle_normal = result.triangle_normal;

                ground_entity_type = entity->type;
                ground_entity_id = entry->key;
                
                found_walkable_point = true;
            }
        }
    }

    if (found_walkable_point) {
        walk_state_result->triangle_normal = triangle_normal;
        walk_state_result->triangle_index = triangle_index;
        walk_state_result->ground_entity_type = ground_entity_type;
        walk_state_result->ground_entity_id = ground_entity_id;
        *grounded_position = highest_point;

#if 1
        add_debug_line(&game_state->debug_state, player_position, highest_point,
                       make_vec4(1.0f, 1.0f, 0.0f, 1.0f));
#endif
    }

    return found_walkable_point;
}

void update_render_state(Render_State *render_state) {
    Camera camera = render_state->camera;
    Mat4 view_matrix = get_view_matrix(camera);
    Mat4 perspective_clip_matrix = make_perspective_clip_matrix(camera.fov_x_degrees, camera.aspect_ratio,
                                                                camera.near, camera.far);
    render_state->view_matrix = view_matrix;
    render_state->perspective_clip_matrix = perspective_clip_matrix;
    render_state->cpv_matrix = perspective_clip_matrix * view_matrix;

    Display_Output display_output = render_state->display_output;
    Mat4 ortho_clip_matrix = make_ortho_clip_matrix((real32) display_output.width,
                                                    (real32) display_output.height,
                                                    0.0f, 100.0f);
    render_state->ortho_clip_matrix = ortho_clip_matrix;
}

Entity *get_entity(Level *level, Entity_Type entity_type, int32 entity_id) {
    Entity *entity = NULL;

    switch (entity_type) {
        case ENTITY_NORMAL:
        {
            Normal_Entity *normal_entity;
            bool32 found = hash_table_find_pointer(level->normal_entity_table,
                                                   entity_id, &normal_entity);
            assert(found);
            entity = (Entity *) normal_entity;
        } break;
        case ENTITY_POINT_LIGHT:
        {
            Point_Light_Entity *point_light_entity;
            bool32 found = hash_table_find_pointer(level->point_light_entity_table,
                                                   entity_id, &point_light_entity);
            assert(found);
            entity = (Entity *) point_light_entity;
        } break;
        default: {
            assert(!"Unhandled entity type.");
        } break;
    }

    return entity;
}

Entity *get_selected_entity(Game_State *game_state) {
    Editor_State *editor_state = &game_state->editor_state;
    Level *level = &game_state->current_level;

    int32 index = editor_state->selected_entity_id;

    Entity *entity = NULL;
    entity = get_entity(level, editor_state->selected_entity_type, index);

    assert(entity);
    return entity;
}

Mesh *get_entity_mesh_pointer(Game_State *game_state, Level *level, Entity *entity) {
    if (entity->type == ENTITY_NORMAL) {
        Normal_Entity *normal_entity = (Normal_Entity *) entity;
        Mesh *mesh = get_mesh_pointer(game_state, level, normal_entity->mesh_type, normal_entity->mesh_id);
        return mesh;
    }

    return NULL;
}

void update_entity_aabb(Game_State *game_state, Level *level, Entity *entity) {
    Mesh *mesh = get_entity_mesh_pointer(game_state, level, entity);
    if (mesh) {
        if (entity->type == ENTITY_NORMAL) {
            Normal_Entity *normal_entity = (Normal_Entity *) entity;
            normal_entity->transformed_aabb = transform_aabb(mesh->aabb, get_model_matrix(entity->transform));
        } else {
            assert(!"Unhandled entity type with mesh and AABB");
        }
    }
}

// TODO: we probably don't always need to update the AABB in some cases; well, idk, there might be uses for AABBs
//       outside of the editor, but that's the only place we're using them right now. although, it is convenient
//       that as long as we use these procedures when transforming entities, the entities will always have an
//       up to date AABB.
void update_entity_position(Game_State *game_state, Entity *entity, Vec3 new_position) {
    entity->transform.position = new_position;

    update_entity_aabb(game_state, &game_state->current_level, entity);

    if (entity->type == ENTITY_NORMAL) {
        Normal_Entity *normal_entity = (Normal_Entity *) entity;
        Collider_Variant *collider = &normal_entity->collider;
        switch (collider->type) {
            case Collider_Type::NONE: break;
            case Collider_Type::CIRCLE: {
                collider->circle.center = new_position;
            } break;
            default: {
                assert(!"Unhandled collider type.");
            } break;
        }
    }
}

void update_entity_rotation(Game_State *game_state, Entity *entity, Quaternion new_rotation) {
    entity->transform.rotation = new_rotation;
    update_entity_aabb(game_state, &game_state->current_level, entity);

    // TODO: modify colliders when rotating
}

void update_entity_scale(Game_State *game_state, Entity *entity, Vec3 new_scale) {
    entity->transform.scale = new_scale;
    update_entity_aabb(game_state, &game_state->current_level, entity);

    // TODO: modify colliders when scaling
#if 0
    if (entity->type == ENTITY_NORMAL) {
        Normal_Entity *normal_entity = (Normal_Entity *) entity;
        Collider_Variant *collider = &normal_entity->collider;
        switch (collider->type) {
            case Collider_Type::NONE: break;
            case Collider_Type::CIRCLE: {
                collider->circle.center = new_position;
            } break;
            default: {
                assert(!"Unhandled collider type.");
            } break;
        }
    }
#endif
}

void set_entity_transform(Game_State *game_state, Entity *entity, Transform transform) {
    entity->transform = transform;
    update_entity_aabb(game_state, &game_state->current_level, entity);
}

Material *get_entity_material(Level *level, Entity *entity) {
    // TODO: maybe just return NULL here instead of asserting
    assert(entity->type == ENTITY_NORMAL);

    // TODO: entities other than normal entities will have materials
    Normal_Entity *normal_entity = (Normal_Entity *) entity;
    Material *selected_material;
    bool32 material_exists = hash_table_find_pointer(level->material_table,
                                                     normal_entity->material_id,
                                                     &selected_material);
    assert(material_exists);
    return selected_material;
}

void set_entity_mesh(Game_State *game_state, Level *level, Entity *entity, Mesh_Type mesh_type, int32 mesh_id) {
    // TODO: we'll probably add entities other than normal entities in the future that have meshes, but for now just check
    //       for ENTITY_NORMAL
    assert(entity->type == ENTITY_NORMAL);
    Mesh mesh = get_mesh(game_state, level, mesh_type, mesh_id);

    switch (entity->type) {
        case ENTITY_NORMAL: {
            Normal_Entity *normal_entity = (Normal_Entity *) entity;
            normal_entity->mesh_type = mesh_type;
            normal_entity->mesh_id = mesh_id;
            normal_entity->transformed_aabb = transform_aabb(mesh.aabb, entity->transform);
        } break;
        default: {
            assert(!"Unhandled entity with mesh type");
        } break;
    }
}

void set_entity_material(Entity *entity, int32 material_id) {
    assert(entity->type == ENTITY_NORMAL);

    switch (entity->type) {
        case ENTITY_NORMAL: {
            Normal_Entity *normal_entity = (Normal_Entity *) entity;
            normal_entity->material_id = material_id;
        } break;
        default: {
            assert(!"Unhandled entity with material type");
        } break;
    }
}

void add_debug_line(Debug_State *debug_state, Vec3 start, Vec3 end, Vec4 color) {
    assert(debug_state->num_debug_lines < MAX_DEBUG_LINES);
    debug_state->debug_lines[debug_state->num_debug_lines++] = { start, end, color };
}

void check_player_collisions(Game_State *game_state) {
    Level *level = &game_state->current_level;
    Player *player = &game_state->player;

    Capsule player_capsule = { player->position,
                               player->position + make_vec3(0.0f, player->height, 0.0f),
                               1.0f };

        
        
    FOR_VALUE_POINTERS(int32, Normal_Entity, level->normal_entity_table) {
        Normal_Entity *entity = value;
        Mesh mesh = get_mesh(game_state, level, entity->mesh_type, entity->mesh_id);
        if (capsule_intersects_mesh(player_capsule, mesh, entity->transform)) {
            char *buf = string_format((Allocator *) &memory.frame_arena, 128,
                                      "capsule is intersecting: %s",
                                      to_char_array((Allocator *) &memory.frame_arena, mesh.name));
            do_text(&game_state->ui_manager, 5.0f, 650.0f, buf, "calibri14", "capsule_intersecting_mesh");
        }
    }
}

void update_player(Game_State *game_state, Controller_State *controller_state,
                   real32 dt) {
    Debug_State *debug_state = &game_state->debug_state;
    Player *player = &game_state->player;

    if (platform_window_has_focus()) {
        real32 delta_x = controller_state->current_mouse.x - controller_state->last_mouse.x;
        real32 delta_y = controller_state->current_mouse.y - controller_state->last_mouse.y;

        real32 heading_delta = 0.2f * delta_x;
        real32 pitch_delta = 0.2f * delta_y;

        int32 heading_rotations = (int32) floorf((player->heading + heading_delta) / 360.0f);
        int32 pitch_rotations = (int32) floorf((player->pitch + pitch_delta) / 360.0f);
        player->heading = (player->heading + heading_delta) - heading_rotations*360.0f;
        player->pitch = clamp(player->pitch + pitch_delta, -90.0f, 90.0f);

        Display_Output display_output = game_state->render_state.display_output;
        Vec2 center = make_vec2(display_output.width / 2.0f, display_output.height / 2.0f);
        platform_set_cursor_pos(center);
        controller_state->current_mouse = center;
    }

    Vec3 player_forward = normalize(truncate_v4_to_v3(make_rotate_matrix(y_axis, player->heading) *
                                                      make_vec4(Player_Constants::forward, 1.0f)));
    Vec3 player_right = normalize(truncate_v4_to_v3(make_rotate_matrix(y_axis, player->heading) *
                                                    make_vec4(Player_Constants::right, 1.0f)));
    if (player->is_grounded) {
        // make basis
        Walk_State *walk_state = &player->walk_state;

        Vec3 forward_point = player->position + player_forward;
        forward_point = get_point_on_plane_from_xz(forward_point.x, forward_point.z,
                                                   walk_state->triangle_normal, player->position);
        player_forward = normalize(forward_point - player->position);

        Vec3 right_point = player->position + player_right;
        right_point = get_point_on_plane_from_xz(right_point.x, right_point.z,
                                                 walk_state->triangle_normal, player->position);
        player_right = normalize(right_point - player->position);

#if 1
        add_debug_line(debug_state,
                       player->position, player->position + player_right, make_vec4(x_axis, 1.0f));
        add_debug_line(debug_state,
                       player->position, player->position + walk_state->triangle_normal, make_vec4(y_axis, 1.0f));
        add_debug_line(debug_state,
                       player->position, player->position + player_forward, make_vec4(z_axis, 1.0f));
#endif
    }

    Vec3 move_vector = make_vec3();
    if (controller_state->key_w.is_down) {
        //move_vector = dot(heading_direction, player_direction) * heading_direction;
        move_vector += player_forward;
    }
    if (controller_state->key_s.is_down) {
        move_vector += -player_forward;
    }
    if (controller_state->key_d.is_down) {
        move_vector += player_right;
    }
    if (controller_state->key_a.is_down) {
        move_vector += -player_right;
    }
    move_vector = normalize(move_vector) * player->speed;

    if (player->is_grounded) {
        player->velocity = move_vector;
    } else {
        player->acceleration = make_vec3(0.0f, -9.81f, 0.0f);
    }

    Vec3 displacement_vector = player->velocity*dt + 0.5f*player->acceleration*dt*dt;
    player->velocity += player->acceleration * dt;

    Vec3 next_position = player->position + displacement_vector;

    Walk_State new_walk_state;
    Vec3 grounded_position;
    bool32 found_new_ground = get_new_walk_state(game_state, player->walk_state, next_position,
                                                 &new_walk_state, &grounded_position);
    if (found_new_ground) {
        player->walk_state = new_walk_state;
        displacement_vector = grounded_position - player->position;
        player->is_grounded = true;
        
        Entity *entity = get_entity(&game_state->current_level,
                                    new_walk_state.ground_entity_type, new_walk_state.ground_entity_id);
        Mesh *mesh = get_entity_mesh_pointer(game_state, &game_state->current_level,
                                             entity);
        assert(mesh);

        Vec3 triangle[3];
        get_triangle(mesh, new_walk_state.triangle_index, triangle);
        
        Mat4 model = get_model_matrix(entity->transform);
        transform_triangle(triangle, &model);

        Vec4 triangle_color = make_vec4(0.0f, 1.0f, 1.0f, 1.0f);
        add_debug_line(debug_state, triangle[0], triangle[1], triangle_color);
        add_debug_line(debug_state, triangle[1], triangle[2], triangle_color);
        add_debug_line(debug_state, triangle[2], triangle[0], triangle_color);
    } else {
        if (player->is_grounded) {
            player->is_grounded = false;
            //player->velocity = make_vec3();
        }
    }

    player->position += displacement_vector;

    //check_player_collisions(game_state);
}

void update_camera(Camera *camera, Vec3 position, real32 heading, real32 pitch, real32 roll) {
    Basis initial_basis = camera->initial_basis;

    camera->heading = heading;
    camera->pitch = pitch;
    camera->roll = roll;

    Mat4 model_matrix = make_rotate_matrix(camera->roll, camera->pitch, camera->heading);
    Vec3 transformed_forward = truncate_v4_to_v3(model_matrix * make_vec4(initial_basis.forward, 1.0f));
    Vec3 transformed_right = truncate_v4_to_v3(model_matrix * make_vec4(initial_basis.right, 1.0f));
    Vec3 transformed_up = cross(transformed_forward, transformed_right);

    Vec3 corrected_right = cross(transformed_up, transformed_forward);
    Vec3 forward = normalize(transformed_forward);
    Vec3 right = normalize(corrected_right);
    Vec3 up = normalize(transformed_up);

    camera->position = position;

    Basis current_basis = { forward, right, up };
    camera->current_basis = current_basis;
}

void add_message(Message_Manager *manager, String text) {
    Message message;
    message.is_deallocated = false;
    message.timer = 0.0f;
    message.text = text;

    Message *messages = manager->messages;
    Message existing = messages[manager->current_message_index];
    if (!existing.is_deallocated && existing.text.allocator) {
        // if we're overwriting, deallocate whatever's there.
        deallocate(messages[manager->current_message_index].text);
        existing.is_deallocated = true;
    }
    messages[manager->current_message_index] = message;
    manager->num_messages++;
    manager->num_messages = min(manager->num_messages, MAX_MESSAGES);
    manager->current_message_index = (manager->current_message_index + 1) % MAX_MESSAGES;
}

void update_messages(Message_Manager *manager, real32 dt) {
    int32 original_num_messages = manager->num_messages;
    int32 index = (MAX_MESSAGES + (manager->current_message_index - 1)) % MAX_MESSAGES;
    Message *messages = manager->messages;

    // sometimes dt can be very large, such as when execution stops due to a dialog being opened, so we cap dt
    // to prevent the message from disappearing instantly after one of those events
    dt = min(1.0f / TARGET_FRAMERATE, dt);

    for (int32 messages_visited = 0; messages_visited < original_num_messages; messages_visited++) {
        messages[index].timer += dt;
        if (messages[index].timer >= manager->message_time_limit) {
            // we have an is_deallocated flag so that in the case where the message array is filled, when
            // one message fades away and is deallocated, we don't deallocate it again if we overwrite it when
            // adding a new message
            manager->num_messages--;

            if (messages[index].text.allocator) {
                deallocate(messages[index].text);
                messages[index].is_deallocated = true;
            }
        }
        index = (MAX_MESSAGES + (index - 1)) % MAX_MESSAGES;
    }
}

void draw_messages(Message_Manager *manager, real32 x_start, real32 y_start) {
    int32 index = (MAX_MESSAGES + (manager->current_message_index - 1)) % MAX_MESSAGES;
    Message *messages = manager->messages;
    real32 y_offset = 30.0f;

    UI_Text_Style text_style;
    // TODO: convert to linear before mixing colors
    text_style.color = make_vec4(1.0f, 1.0f, 1.0f, 1.0f);
    text_style.use_offset_shadow = true;
    text_style.offset_shadow_color = make_vec4(0.0f, 0.0f, 0.0f, 1.0f);
    text_style.text_align_flags = TEXT_JUSTIFY_CENTER;

    Allocator *frame_allocator = (Allocator *) &memory.frame_arena;
    for (int32 messages_visited = 0; messages_visited < manager->num_messages; messages_visited++) {
        Message *message = &messages[index];
        char *string = to_char_array(frame_allocator, message->text);

        real32 total_fade_time = MESSAGE_TIME_LIMIT - MESSAGE_FADE_START;
        real32 unclamped_opacity = 1.0f - ((message->timer - MESSAGE_FADE_START) / total_fade_time);
        real32 opacity = clamp(unclamped_opacity, 0.0f, 1.0f);
        text_style.color.w = opacity;
        text_style.offset_shadow_color.w = opacity;

        do_text(Context::ui_manager, x_start, y_start + messages_visited*y_offset,
                string, "calibri24b", text_style,
                "message_text", index);
                
        index = (MAX_MESSAGES + (index - 1)) % MAX_MESSAGES;
    }
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

    ui_manager->last_frame_active = ui_manager->active;

    if (was_clicked(controller_state->key_f5)) {
        if (game_state->mode == Game_Mode::EDITING) {
            if (!game_state->editor_state.use_freecam) platform_set_cursor_visible(false);
            game_state->mode = Game_Mode::PLAYING;
            Player *player = &game_state->player;
            Camera camera = render_state->camera;
            player->position = render_state->camera.position - make_vec3(0.0f, player->height, 0.0f);
            player->heading = camera.heading;
            player->pitch = camera.pitch;
            player->roll = camera.roll;
            player->velocity = make_vec3();
            // just set is_grounded to false, since we don't have a way to check if the player is on a mesh
            // right now.
            player->is_grounded = false;
        } else {
            game_state->mode = Game_Mode::EDITING;
            if (!game_state->editor_state.use_freecam) {
                platform_set_cursor_visible(true);
            }
        }
    }

    if (game_state->mode == Game_Mode::EDITING) {
        update_editor(game_state, controller_state, dt);
        draw_editor_ui(game_state, controller_state);
        clear_editor_state_for_gone_color_pickers(ui_manager, &game_state->editor_state);
    } else {
        update_player(game_state, controller_state, dt);
        Player player = game_state->player;
        update_camera(&render_state->camera, player.position + make_vec3(0.0f, player.height, 0.0f),
                      player.heading, player.pitch, player.roll);
        update_render_state(render_state);
    }
    
        
    char *buf = (char *) arena_push(&memory.frame_arena, 128);
#if 0
    buf = (char *) arena_push(&memory.frame_arena, 128);
    string_format(buf, 128, "current_mouse: (%f, %f)",
                  controller_state->current_mouse.x, controller_state->current_mouse.y);
    do_text(ui_manager, 0.0f, 24.0f, buf, "times24", "current_mouse_text");
#endif


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
    
    buf = (char *) arena_push(&memory.frame_arena, 128);
    string_format(buf, 128, "pool->blocks_used: %d",
                  memory.string64_pool.blocks_used);
    do_text(ui_manager, 500.0f, 72.0f, buf, "times24", "pool->blocks_used");
    buf = (char *) arena_push(&memory.frame_arena, 128);

#if 0
    buf = (char *) arena_push(&memory.frame_arena, 128);
    string_format(buf, 128, "level string64_pool->first: %p",
                  game_state->current_level.string64_pool_pointer->first);
    do_text(ui_manager, 500.0f, 72.0f, buf, "times24", "pool->first");
    buf = (char *) arena_push(&memory.frame_arena, 128);
#endif

    fill_sound_buffer_with_audio(sound_output, game_state->is_playing_music, &game_state->music, num_samples);

    update_messages(&game_state->message_manager, dt);
    draw_messages(&game_state->message_manager, display_output->width / 2.0f, display_output->height / 2.0f);

    clear_hot_if_gone(ui_manager);
    clear_active_if_gone(ui_manager);

    // NOTE: it's fine to call delete_state_if_gone() here. this won't cause any accesses of deallocated memory
    //       when we render the UI, since if some element isn't in the push_buffer, which is also the condition
    //       that we delete its state, it won't be rendered.
    delete_state_if_gone(ui_manager);
    assert(ui_manager->current_layer == 0);
}
