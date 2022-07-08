#include "platform.h"
#include "memory.h"
#include "entity.h"
#include "game.h"
#include "level.h"

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

bool32 ray_intersects_mesh(Ray ray, Mesh mesh, Transform transform, bool32 include_backside,
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
            assert(!isnan(t));
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
bool32 capsule_intersects_mesh(Capsule capsule, Mesh mesh, Transform transform,
                               Vec3 *penetration_normal, real32 *penetration_depth, Vec3 *intersection_point,
                               int32 *triangle_index_out, Vec3 *triangle_normal_out) {
    Mat4 object_to_world = get_model_matrix(transform);

    uint32 *indices = mesh.indices;

    bool32 intersected = false;
    real32 smallest_penetration_depth = FLT_MIN;
    Vec3 smallest_penetration_normal = {};
    Vec3 smallest_intersection_point = {};
    real32 largest_dot = FLT_MIN;

    Vec3 smallest_penetration_triangle[3] = {};
    int32 triangle_index_result = -1;
    Vec3 triangle_normal_result = {};

    for (int32 i = 0; i < (int32) mesh.num_triangles; i++) {
        Vec3 triangle[3];
        // TODO: don't keep calculating the normal; just calculate it once and pass it down
        get_triangle(&mesh, i, triangle);
        transform_triangle(triangle, &object_to_world);
        Vec3 triangle_normal = get_triangle_normal(triangle);

        // TODO: do we want to return the smallest or largest penetration depth?
        Vec3 normal_result;
        real32 depth_result;
        Vec3 intersection_point_result;
        if (capsule_intersects_triangle(capsule, triangle,
                                        &normal_result, &depth_result, &intersection_point_result)) {
            real32 dot_result = dot(normal_result, triangle_normal);
            // TODO: penetration depth is super confusing, we might just want to return the opposite one, i.e.
            //       the one we use for pushing the capsule out
            // we check for smallest penetration depth since that's what will lead to the largest push out
            // vector (since the penetration vector is from the point on the triangle to the center of the sphere).
            if (!intersected || (dot_result > 0.0f && depth_result < smallest_penetration_depth)) {
            //if (!intersected || depth_result > largest_penetration_depth) {
                intersected = true;
                smallest_penetration_normal = normal_result;
                smallest_penetration_depth = depth_result;
                smallest_penetration_triangle[0] = triangle[0];
                smallest_penetration_triangle[1] = triangle[1];
                smallest_penetration_triangle[2] = triangle[2];
                smallest_intersection_point = intersection_point_result;
                largest_dot = dot_result;
                triangle_index_result = i;
                triangle_normal_result = triangle_normal;
            }
        }
    }

    if (intersected) {
        *penetration_normal = smallest_penetration_normal;
        *penetration_depth = smallest_penetration_depth;
        *intersection_point = smallest_intersection_point;
        *triangle_index_out = triangle_index_result;
        *triangle_normal_out = triangle_normal_result;

#if DEBUG_SHOW_COLLISION_LINES
        Vec4 triangle_color = make_vec4(1.0f, 1.0f, 1.0f, 1.0f);
        add_debug_line(&Context::game_state->debug_state,
                       smallest_penetration_triangle[0], smallest_penetration_triangle[1], triangle_color);
        add_debug_line(&Context::game_state->debug_state,
                       smallest_penetration_triangle[1], smallest_penetration_triangle[2], triangle_color);
        add_debug_line(&Context::game_state->debug_state,
                       smallest_penetration_triangle[2], smallest_penetration_triangle[0], triangle_color);
#endif
    }

    return intersected;
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

void add_debug_line(Debug_State *debug_state, Vec3 start, Vec3 end, Vec4 color) {
    assert(debug_state->num_debug_lines < MAX_DEBUG_LINES);
    debug_state->debug_lines[debug_state->num_debug_lines++] = { start, end, color };
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

#if 0
void draw_messages(Asset_Manager *asset_manager, Message_Manager *manager, real32 x_start, real32 y_start) {
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
    int32 font_id;
    Font font = get_font(asset_manager, "calibri24b", &font_id);
    for (int32 messages_visited = 0; messages_visited < manager->num_messages; messages_visited++) {
        Message *message = &messages[index];
        char *string = to_char_array(frame_allocator, message->text);

        real32 total_fade_time = MESSAGE_TIME_LIMIT - MESSAGE_FADE_START;
        real32 unclamped_opacity = 1.0f - ((message->timer - MESSAGE_FADE_START) / total_fade_time);
        real32 opacity = clamp(unclamped_opacity, 0.0f, 1.0f);
        text_style.color.w = opacity;
        text_style.offset_shadow_color.w = opacity;

        do_text(Context::ui_manager, x_start, y_start + messages_visited*y_offset,
                string, font_id, text_style,
                "message_text", index);
                
        index = (MAX_MESSAGES + (index - 1)) % MAX_MESSAGES;
    }
}
#endif

void init_game(Game_State *game_state,
               Sound_Output *sound_output, uint32 num_samples) {
    game_state->mode = Game_Mode::EDITING;

    Display_Output *display_output = &game_state->render_state.display_output;
    init_editor(&memory.editor_arena, &game_state->editor_state, *display_output);
    Context::editor_state = &game_state->editor_state;

    game_state->player.height = Player_Constants::player_height;

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

    // init message manager
    game_state->message_manager = {};
    game_state->message_manager.message_time_limit = MESSAGE_TIME_LIMIT;
    Context::message_manager = &game_state->message_manager;

    // init asset manager
    Asset_Manager asset_manager = make_asset_manager((Allocator *) &memory.game_data);
    game_state->asset_manager = asset_manager;
    //Context::asset_manager = &game_state->asset_manager;

    // init memory
    uint32 level_arena_size = MEGABYTES(256);
    void *level_arena_base = arena_push(&memory.game_data, level_arena_size, false);
    game_state->level_arena = make_arena_allocator(level_arena_base, level_arena_size);

    // string pool allocators
    //Allocator *string64_allocator = (Allocator *) &memory.string64_pool;
    //Allocator *filename_allocator = (Allocator *) &memory.filename_pool;
    
    // init camera
    init_camera(&game_state->camera, display_output, CAMERA_FOV);

#if 0
    // init fonts
    load_font(asset_manager, "c:/windows/fonts/times.ttf", "times32", 32.0f, 512, 512);
    load_font(asset_manager, "c:/windows/fonts/times.ttf", "times24", 24.0f, 512, 512);

    load_font(asset_manager, "c:/windows/fonts/courbd.ttf", "courier24b", 24.0f, 512, 512);

    load_font(asset_manager, "c:/windows/fonts/calibri.ttf", "calibri14", 14.0f, 512, 512);
    load_font(asset_manager, "c:/windows/fonts/calibrib.ttf", "calibri14b", 14.0f, 512, 512);
    load_font(asset_manager, "c:/windows/fonts/calibrib.ttf", "calibri24b", 24.0f, 512, 512);

    load_font(asset_manager, "c:/windows/fonts/lucon.ttf", "lucidaconsole18", 18.0f, 512, 512);
#endif

    Vec3 origin = make_vec3(0.0f, 10.0f, 0.0f);
    Vec3 direction = make_vec3(0.0f, -1.0f, 0.0f);
    Ray ray = make_ray(origin, direction);
    Transform transform = make_transform();
    transform.scale = make_vec3(1.0f, 0.5f, 1.0f);

    // init ui state
    UI_Manager *ui_manager = &game_state->ui_manager;
    #if 0
    UI_Push_Buffer ui_push_buffer = {};
    ui_push_buffer.size = MEGABYTES(1);
    ui_push_buffer.base = allocate((Allocator *) game_data_arena, ui_push_buffer.size);
    ui_push_buffer.used = 0;
    ui_manager->push_buffer = ui_push_buffer;
    ui_manager->current_layer = 0;
    ui_manager->heap_pointer = &memory.ui_state_heap;
    ui_manager->state_table = make_hash_table<UI_id, UI_Element_State *>((Allocator *) &memory.hash_table_stack,
                                                                         HASH_TABLE_SIZE, &ui_id_equals);
    #else
    ui_manager->allocator = (Allocator *) &memory.ui_state_heap;
    #endif
    Context::ui_manager = ui_manager;

    game_state->is_initted = true;

    Normal_Entity test_entity = {};
    test_entity.mesh_name = make_string("mesh");
    test_entity.material_name = make_string("material");

    Marker m = begin_region();
    Buffer serialized_entity = serialize(temp_region, test_entity);
    end_region(m);
}

Entity *get_entity(Game_State *game_state, int32 id) {
    Game_Level *level = &game_state->level;

    FOR_LIST_NODES(Entity *, level->entities) {
        Entity *entity = current_node->value;
        if (entity->id == id) {
            return entity;
        }
    }

    assert(!"Entity not found.");
    return NULL;
}

Entity *allocate_and_copy_entity(Allocator *allocator, Entity *entity) {
    switch (entity->type) {
        case ENTITY_NORMAL: {
            Normal_Entity *e = (Normal_Entity *) allocate(allocator, sizeof(Normal_Entity));
            *e = copy(allocator, *((Normal_Entity *) entity));
            return (Entity *) e;
        } break;
        case ENTITY_POINT_LIGHT: {
            Point_Light_Entity *e = (Point_Light_Entity *) allocate(allocator, sizeof(Point_Light_Entity));
            *e = copy(allocator, *((Point_Light_Entity *) entity));
            return (Entity *) e;
        } break;
        default: {
            assert(!"Unhandled entity type.");
        }
    }

    return NULL;
}

void load_game_from_editor(Game_State *game_state) {
    Editor_State *editor_state = &game_state->editor_state;
    Editor_Level *editor_level = &editor_state->level;

    if (!editor_state->use_freecam) platform_set_cursor_visible(false);

    game_state->asset_manager = editor_state->asset_manager;
    
    Arena_Allocator *level_arena = &game_state->level_arena;
    Game_Level *game_level = &game_state->level;

    clear_arena(level_arena);
    make_and_init_linked_list(Entity *, &game_level->entities, (Allocator *) level_arena);

    FOR_LIST_NODES(Entity *, editor_level->entities) {
        Entity *copied_entity = allocate_and_copy_entity((Allocator *) level_arena, current_node->value);
        add(&game_level->entities, copied_entity);
    }

    Player *player = &game_state->player;
    Camera camera = editor_state->camera;
    player->position = camera.position - make_vec3(0.0f, player->height, 0.0f);
    player->heading = camera.heading;
    player->pitch = camera.pitch;
    player->roll = camera.roll;
    player->velocity = make_vec3();
    player->is_grounded = false;
}

inline void get_transformed_triangle(Mesh *mesh, int32 triangle_index, Mat4 *object_to_world,
                                     Vec3 transformed_triangle[3]) {
    get_triangle(mesh, triangle_index, transformed_triangle);
    transform_triangle(transformed_triangle, object_to_world);
}

bool32 move_player_to_closest_ground(Game_State *game_state, real32 max_drop_distance) {
    Game_Level *level = &game_state->level;
    Asset_Manager *asset_manager = &game_state->asset_manager;

    Player *player = &game_state->player;

    // NOTE: since we're using a ray to find the ground, it is possible that we can get stuck if our level
    //       has some type of chasm that doesn't fit the player, but has ground that was close enough that
    //       we moved the player to the bottom of the chasm. just don't put that type of geometry in levels.
    
    Ray ray = make_ray(player->position, -y_axis);
    real32 t_min = FLT_MAX;
    bool32 found_ground = false;
    
    FOR_LIST_NODES(Entity *, level->entities) {
        Entity *uncast_entity = current_node->value;
        if (uncast_entity->type != ENTITY_NORMAL) continue;

        Normal_Entity *entity = (Normal_Entity *) current_node->value;
        Mat4 object_to_world = get_model_matrix(entity->transform);
        Mesh mesh = get_mesh(asset_manager, entity->mesh_id);

        // TODO: check against AABB first?
        Ray_Intersects_Mesh_Result result;
        if (ray_intersects_mesh(ray, mesh, entity->transform, false, &result)) {
            if (result.t < t_min && result.t <= max_drop_distance) {
                t_min = result.t;
                found_ground = true;
            }
        }
    }

    if (found_ground) {
        player->position = ray.origin + ray.direction*t_min;
        player->is_grounded = true;
    }

    return found_ground;
}

inline bool32 hit_bottom_player_capsule_sphere(Capsule *capsule, Vec3 *point_on_capsule) {
    real32 relative_height_from_base = fabsf(point_on_capsule->y - capsule->base.y);
    // we subtract a bit from the capsule radius so that we don't end up walking on walls (i.e. when the
    // intersection point is right at the transition point from the bottom cap and the cylinder)
    return relative_height_from_base < (capsule->radius - 0.1f);
}

void do_collisions(Game_State *game_state, Vec3 initial_move) {
    Game_Level *level = &game_state->level;
    Asset_Manager *asset_manager = &game_state->asset_manager;

    Player *player = &game_state->player;

    player->position += initial_move;

    bool32 found_ground = false;
    bool32 intersected = false;

    // we use the initial_move vector here, since using the correction displacement from a previous
    // intersection can result in behaviour such as pushing the player through meshes
    // (you can see this if you try pushing yourself underneath a sloped plane made up of 2
    // triangles)
    Vec3 displacement = initial_move;
    real32 displacement_length = distance(initial_move);
    assert(displacement_length > EPSILON);
    
    for (int32 num_collisions = 0; num_collisions < MAX_FRAME_COLLISIONS; num_collisions++) {
        Capsule player_capsule = make_capsule(player->position,
                                              player->position + make_vec3(0.0f, player->height, 0.0f),
                                              Player_Constants::capsule_radius);

        FOR_LIST_NODES(Entity *, level->entities) {
            Entity *uncast_entity = current_node->value;
            if (uncast_entity->type != ENTITY_NORMAL) continue;

            Normal_Entity *entity = (Normal_Entity *) current_node->value;
            Mat4 object_to_world = get_model_matrix(entity->transform);
            Mesh *mesh = get_mesh_pointer(asset_manager, entity->mesh_id);

            for (int32 triangle_index = 0; triangle_index < (int32) mesh->num_triangles; triangle_index++) {
                Vec3 triangle[3];
                get_transformed_triangle(mesh, triangle_index, &object_to_world, triangle);

                Vec3 triangle_normal = get_triangle_normal(triangle);

                Vec3 penetration_normal, penetration_point;
                real32 penetration_depth;

                intersected = capsule_intersects_triangle(player_capsule, triangle,
                                                          &penetration_normal,
                                                          &penetration_depth,
                                                          &penetration_point);

                // TODO: there is a bug in this code when sliding under and along two adjacent triangles above you.
                //       since the adjacent triangles are not being treated as a single plane, you can slide along
                //       one, but then get caught on the other when you hit it. if the second triangle were the only
                //       one there, this would be correct behaviour.
                if (intersected) {
#if 1
                    if (dot(initial_move, triangle_normal) < 0.0f) {
                        Vec3 normalized_displacement = displacement / displacement_length;
                        Vec3 normalized_correction = (-penetration_normal *
                                                      dot(normalized_displacement, penetration_normal));

                        // TODO: this causes jittering when pushing into things, but we want to do it this way since
                        //       just pushing out by the penetration depth won't give you the correct speed when
                        //       sliding along slopes or walls. also, this causes us to move out too much when
                        //       falling into the ground. we get pushed out of the ground, then we attempt to
                        //       collide with the ground again with gravity so that we don't keep alternating
                        //       is_grounded, but with this uncommented, we get pushed out so much that the displacement
                        //       from gravity or some small displacement we use to push us downwards isn't enough for
                        //       us to collide again with the ground, and so we get the undesirable behaviour of
                        //       is_grounded constantly alternating while walking on a surface.
                        //player->position += displacement_length*normalized_correction;
                        player->position += penetration_normal * (penetration_depth + 0.00001f);

                        // we check this so that we can nicely climb up triangles that are low enough without
                        // being pushed brought down by gravity. this is also necessary since if we didn't have this,
                        // if a wall triangle comes before a floor triangle, then we would get pushed out before
                        // we'd even be able to intersect with the floor and walk on the floor.
                        if (hit_bottom_player_capsule_sphere(&player_capsule, &penetration_point)) {
                            found_ground = true;
                            player->walk_state.triangle_normal = triangle_normal;
                        }

                        player_capsule = make_capsule(player->position,
                                                      player->position + make_vec3(0.0f, player->height, 0.0f),
                                                      Player_Constants::capsule_radius);

                        // we don't early-out here since we might encounter a walkable surface later, on this same
                        // mesh.
                        //break;
                    }
#endif

#if 0
                    // TODO: we don't actually do any correction based on displacement since 
                    player->position += penetration_normal * (penetration_depth + 0.00001f);

                    // TODO: this can lead to unexpected results
                    if (triangle_normal.y > 0.3f) {
                        player->walk_state.triangle_normal = triangle_normal;
                        player->walk_state.triangle_index = triangle_index;
                        player->walk_state.ground_entity_id = entity->id;
                        found_ground = true;
                    }
#endif

#if DEBUG_SHOW_COLLISION_LINES
                    Vec4 triangle_color = make_vec4(1.0f, 1.0f, 1.0f, 1.0f);
                    add_debug_line(&game_state->debug_state,
                                   triangle[0], triangle[1], triangle_color);
                    add_debug_line(&game_state->debug_state,
                                   triangle[1], triangle[2], triangle_color);
                    add_debug_line(&game_state->debug_state,
                                   triangle[2], triangle[0], triangle_color);
#endif
                }
            }

            // TODO: not sure if we should keep this early-out
            if (intersected) break;
        }

        if (!intersected) break;
    }

    bool32 was_grounded = player->is_grounded && !found_ground;
    
    player->is_grounded = found_ground;
}

void update_player(Game_State *game_state, Controller_State *controller_state,
                   real32 dt) {
    Debug_State *debug_state = &game_state->debug_state;
    Asset_Manager *asset_manager = &game_state->asset_manager;
    Player *player = &game_state->player;

    dt = min(1.0f / TARGET_FRAMERATE, dt);

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

    Vec3 rotated_player_forward = normalize(truncate_v4_to_v3(make_rotate_matrix(y_axis, player->heading) *
                                                              make_vec4(Player_Constants::forward, 1.0f)));
    Vec3 rotated_player_right = normalize(truncate_v4_to_v3(make_rotate_matrix(y_axis, player->heading) *
                                                            make_vec4(Player_Constants::right, 1.0f)));
    Vec3 player_forward, player_right;
    orthonormalize(rotated_player_forward, rotated_player_right, &player_forward, &player_right);

    real32 normal_proj_y_axis = dot(player->walk_state.triangle_normal, y_axis);
    // between 0 and 45 degrees from the y axis
    real32 one_over_root_two = 0.7071067f;
    bool32 slope_is_shallow = normal_proj_y_axis >= one_over_root_two && normal_proj_y_axis <= 1.0f;
    
    if (player->is_grounded && slope_is_shallow) {
        // make basis
        Walk_State walk_state = player->walk_state;

        Vec3 forward_point = player->position + player_forward;
        forward_point = get_point_on_plane_from_xz(forward_point.x, forward_point.z,
                                                   walk_state.triangle_normal, player->position);
        player_forward = normalize(forward_point - player->position);

        Vec3 right_point = player->position + player_right;
        right_point = get_point_on_plane_from_xz(right_point.x, right_point.z,
                                                 walk_state.triangle_normal, player->position);
        player_right = normalize(right_point - player->position);

#if DEBUG_SHOW_WALK_BASIS
        add_debug_line(debug_state,
                       player->position, player->position + player_right, make_vec4(x_axis, 1.0f));
        add_debug_line(debug_state,
                       player->position, player->position + walk_state.triangle_normal, make_vec4(y_axis, 1.0f));
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

    Vec3 gravity_acceleration = make_vec3(0.0f, -9.81f, 0.0f);
    
    if (player->is_grounded) {
        player->velocity = move_vector;
        player->acceleration = {};
        Vec3 displacement_vector = player->velocity*dt;

        bool32 initial_is_grounded = player->is_grounded;
        
        if (distance(displacement_vector) > EPSILON) {
            // TODO: rename do_collisions - do_collisions actually does the move given by displacement_vector
            do_collisions(game_state, displacement_vector);
            if (!player->is_grounded) {
                // since collisions push out the player capsule so that they aren't colliding anymore, if we base
                // is_grounded simply on whether we're colliding with an eligible ground triangle (i.e. a triangle
                // that intersects with the capsule's bottom cap), we will constantly be cycling between is_grounded
                // being true and it being false. this is undesirable since it prevents us from easily determining
                // whether the player actually is no longer walking on a triangle - we use this fact to drop the
                // player down to a close enough triangle when they walk off a triangle so that we can walk onto
                // downward slopes smoothly. without this, they would move off a triangle, then have gravity move
                // them downwards until they hit the triangle, which feels weird. so therefore, after we do the
                // first do_collisions, we then call do_collisions again with some small negative y displacement
                // so that we can become grounded again after being pushed out.
                // TODO: there may be a more efficient way of doing this, but this works for now.
                
                // don't use player velocity in displacement here, since we already moved by that in the previous
                // do_collisions() call
                displacement_vector = make_vec3(0.0f, -0.001f, 0.0f);
                do_collisions(game_state, displacement_vector);
            }
        }

        bool32 was_grounded = initial_is_grounded && !player->is_grounded;

        if (was_grounded) {
            Vec3 before_drop_move_position = player->position;
            bool32 moved = move_player_to_closest_ground(game_state, MAX_DROP_DISTANCE);
            if (moved) {
                Vec3 drop_move_delta = player->position - before_drop_move_position;
                Vec3 drop_move_direction = normalize(drop_move_delta);
                player->position = before_drop_move_position + drop_move_direction*player->speed*dt;
            }
        }
    } else {
        player->acceleration = gravity_acceleration;
        Vec3 displacement_vector = player->velocity*dt + 0.5f*player->acceleration*dt*dt;
        player->velocity += player->acceleration * dt;

        if (distance(displacement_vector) > EPSILON) {
            do_collisions(game_state, displacement_vector);
        }
    }
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

void update_game(Game_State *game_state, Controller_State *controller_state, Sound_Output *sound_output,
                 real32 dt) {
    // NOTE: we assume that game_state has already been initted
    update_player(game_state, controller_state, dt);

    Player *player = &game_state->player;
    update_camera(&game_state->camera, &game_state->render_state.display_output,
                  player->position + make_vec3(0.0f, player->height, 0.0f),
                  player->heading, player->pitch, player->roll);
    update_render_state(&game_state->render_state, game_state->camera);
}

// TODO: try implementing buttons
void draw_test_ui(UI_Manager *ui, Display_Output *display_output) {
    ui_frame_init(ui, display_output);

    ui_push_position(ui, { 5.0f, 5.0f });
    ui_push_size_type(ui, UI_SIZE_FIT_CHILDREN);
    ui_push_background_color(ui, { 1.0f, 0.0f, 0.0f, 1.0f });
    ui_push_widget(ui, make_ui_id("test"), UI_WIDGET_DRAW_BACKGROUND);

    ui_push_size_type(ui, UI_SIZE_ABSOLUTE);
    ui_push_position(ui, { 100.0f, 500.0f });
    ui_push_size(ui, { 50.0f, 50.0f });
    ui_push_background_color(ui, { 0.0f, 0.0f, 1.0f, 1.0f });
    ui_add_widget(ui, make_ui_id("test2"), UI_WIDGET_DRAW_BACKGROUND);

    ui_push_size(ui, { 100.0f, 80.0f });
    ui_push_background_color(ui, { 0.0f, 1.0f, 0.0f, 1.0f });
    ui_add_widget(ui, make_ui_id("test3"), UI_WIDGET_DRAW_BACKGROUND);
    
    ui_calculate_ancestor_dependent_sizes(ui);
    ui_calculate_child_dependent_sizes(ui);
    ui_calculate_positions(ui);
}

void update(Game_State *game_state,
            Controller_State *controller_state,
            Sound_Output *sound_output, uint32 num_samples) {
    Display_Output *display_output = &game_state->render_state.display_output;
    if (!game_state->is_initted) {
        debug_print("initting game %d!\n", 123);
        init_game(game_state, sound_output, num_samples);
        //return;
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

    if (platform_window_has_focus() && was_clicked(controller_state->key_f5)) {
        if (game_state->mode == Game_Mode::EDITING) {
            load_game_from_editor(game_state);
            game_state->mode = Game_Mode::PLAYING;
        } else {
            game_state->mode = Game_Mode::EDITING;

            if (!game_state->editor_state.use_freecam) {
                platform_set_cursor_visible(true);
            }

            // debug
            FOR_LIST_NODES(Entity *, game_state->editor_state.level.entities) {
                Entity *entity = current_node->value;
                if (entity->type == ENTITY_NORMAL) {
                    Normal_Entity *e = (Normal_Entity *) entity;
                    if (e->collider.type == Collider_Type::CAPSULE) {
                        update_entity_position(&game_state->editor_state.asset_manager, entity, game_state->player.position);
                    }
                }
            }
        }
    }

    UI_Manager *ui_manager = &game_state->ui_manager;
    Render_State *render_state = &game_state->render_state;

    ui_manager->last_frame_active = ui_manager->active;

    //update_render_state(render_state);
    
    Asset_Manager *asset_manager;

    if (game_state->mode == Game_Mode::EDITING) {
        asset_manager = &game_state->editor_state.asset_manager;
        update_editor(game_state, controller_state, dt);
        //draw_editor(game_state, controller_state);
        game_state->editor_state.is_startup = false;
    } else {
        asset_manager = &game_state->asset_manager;
        update_game(game_state, controller_state, sound_output, dt);
    }

    // TODO: walk through this in the debugger
    draw_test_ui(ui_manager, display_output);
    
    Player *player = &game_state->player;

    #if 0
    int32 font_id;
    Font font = get_font(asset_manager, "calibri14", &font_id);

    char *dt_string = string_format((Allocator *) &memory.frame_arena, "FPS %d / dt %.3f", 
                                    (int32) round(game_state->last_second_fps), dt);
    do_text(ui_manager, 5.0f, 14.0f, dt_string, font_id, "dt_string");
    #endif

    fill_sound_buffer_with_audio(sound_output, game_state->is_playing_music, &game_state->music, num_samples);

    #if 0
    update_messages(&game_state->message_manager, dt);
    draw_messages(asset_manager, &game_state->message_manager,
                  display_output->width / 2.0f, display_output->height / 2.0f);
    #endif
    
    //clear_hot_if_gone(ui_manager);
    //clear_active_if_gone(ui_manager);

    // NOTE: it's fine to call delete_state_if_gone() here. this won't cause any accesses of deallocated memory
    //       when we render the UI, since if some element isn't in the push_buffer, which is also the condition
    //       that we delete its state, it won't be rendered.
    //delete_state_if_gone(ui_manager);
    //assert(ui_manager->current_layer == 0);
}
