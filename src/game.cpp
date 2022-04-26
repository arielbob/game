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

    Vec3 origin = make_vec3(0.0f, 10.0f, 0.0f);
    Vec3 direction = make_vec3(0.0f, -1.0f, 0.0f);
    Ray ray = make_ray(origin, direction);
    Transform transform = make_transform();
    transform.scale = make_vec3(1.0f, 0.5f, 1.0f);

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

Entity *get_entity(Level *level, Entity_Type entity_type, int32 entity_index) {
    Entity *entity = NULL;

    switch (entity_type) {
        case ENTITY_NORMAL:
        {
            entity = (Entity *) &level->normal_entities[entity_index];
        } break;
        case ENTITY_POINT_LIGHT:
        {
            entity = (Entity *) &level->point_lights[entity_index];
        } break;
        default: {
            assert(!"Unhandled entity type.");
        } break;
    }

    return entity;
}

Entity *get_selected_entity(Game_State *game_state) {
    Editor_State *editor_state = &game_state->editor_state;

    
    int32 index = editor_state->selected_entity_index;

    Level *level = &game_state->current_level;

    Entity *entity = NULL;
    entity = get_entity(level, editor_state->selected_entity_type, index);

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

void add_debug_line(Debug_State *debug_state, Vec3 start, Vec3 end, Vec4 color) {
    assert(debug_state->num_debug_lines < MAX_DEBUG_LINES);
    debug_state->debug_lines[debug_state->num_debug_lines++] = { start, end, color };
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
        Vec3 forward_point = player->position + player_forward;
        forward_point = get_point_on_plane_from_xz(forward_point.x, forward_point.z,
                                                   player->triangle_normal, player->position);
        player_forward = normalize(forward_point - player->position);

        Vec3 right_point = player->position + player_right;
        right_point = get_point_on_plane_from_xz(right_point.x, right_point.z,
                                                 player->triangle_normal, player->position);
        player_right = normalize(right_point - player->position);

        add_debug_line(debug_state,
                       player->position, player->position + player_right, make_vec4(x_axis, 1.0f));
        add_debug_line(debug_state,
                       player->position, player->position + player->triangle_normal, make_vec4(y_axis, 1.0f));
        add_debug_line(debug_state,
                       player->position, player->position + player_forward, make_vec4(z_axis, 1.0f));
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
        Vec3 current_triangle[3];
        Entity *ground_entity = get_entity(&game_state->current_level,
                                           player->ground_entity_type, player->ground_entity_index);
        Mesh *ground_mesh = get_mesh_pointer(game_state, &game_state->current_level,
                                             ground_entity->mesh_type, ground_entity->mesh_id);
        get_triangle(ground_mesh, player->triangle_index, current_triangle);
        Mat4 ground_model_matrix = get_model_matrix(ground_entity->transform);
        transform_triangle(current_triangle, &ground_model_matrix);

        Vec3 point_on_triangle;
        if (!get_point_on_triangle_from_xz(player->position.x, player->position.z,
                                           current_triangle, &point_on_triangle)) {
            player->is_grounded = false;
            player->velocity = make_vec3();
        }

        char *buf = string_format((Allocator *) &memory.frame_arena, 64,
                                  "current triangle index: %d", player->triangle_index);
        do_text(&game_state->ui_manager, 5.0f, 700.0f, buf, "calibri14", "current_triangle_index");
    }

    if (player->is_grounded) {
        player->velocity = move_vector;
    } else {
        player->acceleration = make_vec3(0.0f, -9.81f, 0.0f);
    }

    Vec3 displacement_vector = player->velocity*dt + 0.5f*player->acceleration*dt*dt;
    player->velocity += player->acceleration * dt;

    if (!player->is_grounded) {
        Level *level = &game_state->current_level;
        real32 t_min = FLT_MAX;
        bool32 intersected = false;
        Vec3 intersected_triangle_normal = make_vec3();
        int32 intersected_triangle_index = -1;
        int32 ground_entity_index = -1;
        Entity_Type ground_entity_type = ENTITY_NONE;
        
        for (int32 i = 0; i < level->num_normal_entities; i++) {
            real32 aabb_t;
            Normal_Entity *entity = &level->normal_entities[i];
            if (entity->is_walkable) {
                Mesh mesh = get_mesh(game_state, level, entity->mesh_type, entity->mesh_id);
                Ray displacement_ray = make_ray(player->position, displacement_vector);
                if (ray_intersects_aabb(displacement_ray, entity->transformed_aabb, &aabb_t) && (aabb_t < t_min)) {
                    // we check for t < 1.0f, since we only want the intersections that are inside the
                    // displacement line
                    Ray_Intersects_Mesh_Result result;
                    if (ray_intersects_mesh(displacement_ray, mesh, entity->transform, false, &result) &&
                        (result.t < 1.0f) && (result.t < t_min)) {
                        t_min = result.t;
                        intersected = true;
                        intersected_triangle_normal = result.triangle_normal;
                        intersected_triangle_index = result.triangle_index;
                        ground_entity_index = i;
                        ground_entity_type = ENTITY_NORMAL;
                    }
                }
            }
        }

        if (intersected) {
            displacement_vector *= t_min;
            player->acceleration = make_vec3();
            player->velocity = make_vec3();
            player->is_grounded = true;

            player->triangle_normal = intersected_triangle_normal;
            player->triangle_index = intersected_triangle_index;
            player->ground_entity_index = ground_entity_index;
            player->ground_entity_type = ground_entity_type;
        }
    }

    player->position += displacement_vector;
}

#if 0
void update_player(Game_State *game_state, Controller_State *controller_state,
                   real32 dt) {
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

#if 0
    Mat4 rotate_matrix = get_rotate_matrix_from_euler_angles(player->roll, player->pitch, player->heading);
    Vec3 player_direction = normalize(truncate_v4_to_v3(rotate_matrix *
                                                        make_vec4(Player_Constants::forward, 1.0f)));
#endif
    Vec3 displacement_vector = make_vec3();
    if (controller_state->key_w.is_down) {
        //displacement_vector = dot(heading_direction, player_direction) * heading_direction;
        displacement_vector += player_forward;
    }
    if (controller_state->key_s.is_down) {
        displacement_vector += -player_forward;
    }
    if (controller_state->key_d.is_down) {
        displacement_vector += player_right;
    }
    if (controller_state->key_a.is_down) {
        displacement_vector += -player_right;
    }

    displacement_vector = normalize(displacement_vector) * player->speed * dt;

#if 0
    if (distance(displacement_vector) > player->speed) {
        displacement_vector = normalize(displacement_vector) * player->speed;
    }
#endif

    if (player->is_grounded) {
        player->velocity = displacement_vector;
        player->position += player->velocity;
    } else {

    }

#if 1
    Vec3 v0 = player->velocity;
    Vec3 gravity = make_vec3(0.0f, -9.8f, 0.0f);
    if (!player->is_grounded) player->acceleration = gravity;
    Vec3 player_displacement = v0 + 0.5f*player->acceleration*dt*dt;
    player->velocity = v0 + player->acceleration * dt;

    Level *level = &game_state->current_level;
    real32 t_min = FLT_MAX;
    bool32 intersected = false;
    for (int32 i = 0; i < level->num_normal_entities; i++) {
        real32 t, aabb_t;
        Normal_Entity *entity = &level->normal_entities[i];
        if (entity->is_walkable) {
            Mesh mesh = get_mesh(game_state, level, entity->mesh_type, entity->mesh_id);
            Ray displacement_ray = make_ray(player->position, player_displacement);
            if (ray_intersects_aabb(displacement_ray, entity->transformed_aabb, &aabb_t) && (aabb_t < t_min)) {
                // we check for t < 1.0f, since we only want the intersections that are inside the
                // displacement line
                if (ray_intersects_mesh(displacement_ray, mesh, entity->transform, &t) &&
                    (t < 1.0f) && (t < t_min)) {
                    t_min = t;
                    intersected = true;
                }
            }
        }
    }

    if (intersected) {
        char *intersect_text = string_format((Allocator *) &memory.frame_arena, 64, "intersect t: %f", t_min);
        do_text(&game_state->ui_manager, 5.0f, 28.0f, intersect_text, "calibri14", "intersect_t");
    } else {
        char *intersect_text = string_format((Allocator *) &memory.frame_arena, 64, "intersect t: None");
        do_text(&game_state->ui_manager, 5.0f, 28.0f, intersect_text, "calibri14", "intersect_t");
    }
    
    if (intersected) {
        player->position += player_displacement * t_min;
        player->acceleration = make_vec3();
        player->velocity = make_vec3();
        player->is_grounded = true;
    } else {
        player->position += player_displacement;
    }
#endif


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
    move_vector = normalize(move_vector) * player->speed * dt;

    //Vec3 displacement_vector;
    Vec3 displacement_vector;

    if (player->is_grounded) {
        displacement_vector = move_vector;
    } else {
        displacement_vector = player->velocity * 0.5f*player->acceleration*dt*dt;
    }

    player->velocity += player->acceleration * dt;

    if (!is_grounded) {
        Level *level = &game_state->current_level;
        real32 t_min = FLT_MAX;
        bool32 intersected = false;
        for (int32 i = 0; i < level->num_normal_entities; i++) {
            real32 t, aabb_t;
            Normal_Entity *entity = &level->normal_entities[i];
            if (entity->is_walkable) {
                Mesh mesh = get_mesh(game_state, level, entity->mesh_type, entity->mesh_id);
                Ray displacement_ray = make_ray(player->position, displacement_vector);
                if (ray_intersects_aabb(displacement_ray, entity->transformed_aabb, &aabb_t) && (aabb_t < t_min)) {
                    // we check for t < 1.0f, since we only want the intersections that are inside the
                    // displacement line
                    if (ray_intersects_mesh(displacement_ray, mesh, entity->transform, &t) &&
                        (t < 1.0f) && (t < t_min)) {
                        t_min = t;
                        intersected = true;
                    }
                }
            }
        }

        if (intersected) {
            player->position += displacement_vector * t_min;
            player->acceleration = make_vec3();
            player->velocity = make_vec3();
            player->is_grounded = true;
        } else {
            player->position += displacement_vector;
        }
    }
    
}
#endif

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
    
    fill_sound_buffer_with_audio(sound_output, game_state->is_playing_music, &game_state->music, num_samples);

    clear_hot_if_gone(ui_manager);
    delete_state_if_gone(ui_manager);
    assert(ui_manager->current_layer == 0);
}
