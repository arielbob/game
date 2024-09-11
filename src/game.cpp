#include "platform.h"
#include "memory.h"
#include "game.h"
#include "level.h"

global_variable int32 samples_written_2;

void init_asset_manager(Arena_Allocator *game_data) {
    uint32 heap_size = ASSET_HEAP_SIZE;
    void *heap_base = arena_push(game_data, heap_size);

    // TODO: i don't think this needs to be a pointer, really - we can just store the object like we do
    //       with the level heap in the function below
    Heap_Allocator *heap = (Heap_Allocator *) arena_push(game_data, sizeof(Heap_Allocator));
    *heap = make_heap_allocator(heap_base, heap_size);

    *asset_manager = {};
    asset_manager->allocator = (Allocator *) heap;

    // init asset update queues
    init_asset_update_queue(game_data, &asset_manager->mesh_update_queue);
    init_asset_update_queue(game_data, &asset_manager->texture_update_queue);
    init_asset_update_queue(game_data, &asset_manager->animation_update_queue);
}

void deinit_asset_manager() {
    // deinit asset update queues
    deinit_asset_update_queue(&asset_manager->mesh_update_queue);
    deinit_asset_update_queue(&asset_manager->texture_update_queue);
    deinit_asset_update_queue(&asset_manager->animation_update_queue);
}

void init_level(Level *level, Arena_Allocator *game_data) {
    // this should only be called when we're initting the game
    assert(!game_state->is_initted);
    
    uint32 heap_size = LEVEL_HEAP_SIZE;
    void *heap_base = arena_push(game_data, heap_size);

    *level = {};
    level->heap = make_heap_allocator(heap_base, heap_size);
}

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

inline bool32 just_pressed_or_repeated(Controller_Button_State button_state) {
    return ((button_state.is_down && !button_state.was_down) || button_state.repeat);
}

inline bool32 just_lifted(Controller_Button_State button_state) {
    return (!button_state.is_down && button_state.was_down);
}

inline Vec2 get_mouse_delta() {
    //return platform_get_cursor_pos() - Context::controller_state->last_mouse;
    return (Context::controller_state->current_mouse - Context::controller_state->last_mouse);
}

Vec3 cursor_pos_to_world_space(Vec2 cursor_pos) {
    Display_Output display_output = render_state->display_output;
    
    Mat4 cpv_matrix_inverse = inverse(render_state->cpv_matrix);

    Vec4 clip_space_coordinates = { 2.0f * (cursor_pos.x / display_output.width) - 1.0f,
        -2.0f * (cursor_pos.y / display_output.height) + 1.0f,
        -1.0f, 1.0f };

    Vec4 cursor_world_space_homogeneous = cpv_matrix_inverse * clip_space_coordinates;
    Vec3 cursor_world_space = homogeneous_divide(cursor_world_space_homogeneous);

    return cursor_world_space;
}

bool32 ray_intersects_mesh(Ray ray, Mesh *mesh, Transform transform, bool32 include_backside,
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

    uint32 *indices = mesh->indices;

    // this might be very slow - this is very cache unfriendly since we're constantly jumping around the
    // vertices array, which in some cases is large.
    // we could cache every single triangle vertex in an array of size num_triangles*3, so our
    // suzanne mesh with 62976 triangles would take up 62976*3*sizeof(real32) = 755712 bytes = 756KB,
    // which isn't terrible, but i'm not sure we even have a need for a fast ray_intersects_mesh()
    // right now, so this way of doing it is fine for now.
    real32 t_min = FLT_MAX;
    bool32 hit = false;
    int32 hit_triangle_index = -1;
    for (int32 i = 0; i < (int32) mesh->num_triangles; i++) {
        Vec3 triangle[3];
        triangle[0] = get_vertex_from_index(mesh, indices[i * 3]);
        triangle[1] = get_vertex_from_index(mesh, indices[i * 3 + 1]);
        triangle[2] = get_vertex_from_index(mesh, indices[i * 3 + 2]);
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
        Vec3 v1 = get_vertex_from_index(mesh, indices[hit_triangle_index * 3]);
        Vec3 v2 = get_vertex_from_index(mesh, indices[hit_triangle_index * 3 + 1]);
        Vec3 v3 = get_vertex_from_index(mesh, indices[hit_triangle_index * 3 + 2]);

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
        real32 depth_result, distance_from_plane;
        Vec3 intersection_point_result;
        if (capsule_intersects_triangle(capsule, triangle,
                                        &normal_result, &depth_result, &intersection_point_result,
                                        &distance_from_plane)) {
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
        add_debug_line(&game_state->debug_state,
                       smallest_penetration_triangle[0], smallest_penetration_triangle[1], triangle_color);
        add_debug_line(&game_state->debug_state,
                       smallest_penetration_triangle[1], smallest_penetration_triangle[2], triangle_color);
        add_debug_line(&game_state->debug_state,
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

void draw_messages(Message_Manager *manager, real32 y_start) {
    int32 index = (MAX_MESSAGES + (manager->current_message_index - 1)) % MAX_MESSAGES;
    Message *messages = manager->messages;
    real32 y_offset = 30.0f;

    Allocator *frame_allocator = (Allocator *) &memory.frame_arena;

    ui_push_existing_widget(ui_manager->always_on_top_layer);
    
    UI_Theme message_container = NULL_THEME;
    message_container.position_type = UI_POSITION_FLOAT;
    message_container.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN };
    message_container.semantic_size = { 1.0f, 0.0f };
    message_container.layout_type = UI_LAYOUT_CENTER;
    
    UI_Theme text_theme = NULL_THEME;
    text_theme.size_type = { UI_SIZE_FIT_TEXT, UI_SIZE_FIT_TEXT };
    text_theme.text_color = make_vec4(1.0f, 1.0f, 1.0f, 1.0f);
    text_theme.font = "calibri24b";
    text_theme.background_color = make_vec4(0.0f, 0.0f, 0.0f, 0.5f);
    
    for (int32 messages_visited = 0; messages_visited < manager->num_messages; messages_visited++) {
        Message *message = &messages[index];
        char *string = to_char_array(frame_allocator, message->text);

        real32 total_fade_time = MESSAGE_TIME_LIMIT - MESSAGE_FADE_START;
        real32 unclamped_opacity = 1.0f - ((message->timer - MESSAGE_FADE_START) / total_fade_time);
        real32 opacity = clamp(unclamped_opacity, 0.0f, 1.0f);

        // TODO: does this need to be done in linear-space?
        text_theme.text_color.w = opacity;
        text_theme.background_color.w *= opacity;

        message_container.semantic_position = { 0.0f, y_start + messages_visited*y_offset };
        ui_add_and_push_widget("", message_container, UI_WIDGET_DRAW_BACKGROUND);
        {
            do_text(string, text_theme, UI_WIDGET_DRAW_BACKGROUND);
        }
        ui_pop_widget();
        
        index = (MAX_MESSAGES + (index - 1)) % MAX_MESSAGES;
    }

    ui_pop_widget();
}

void init_player() {
    Player *player = &game_state->player;
    *player = {};
    player->height = Player_Constants::player_height;
    player->speed = Player_Constants::initial_speed;

    player->position.y = 10.0f;
}

void reset_player(Level *level) {
    init_player();

    Player *player = &game_state->player;
    Spawn_Point spawn_point = level->spawn_point;

    player->position = spawn_point.position;
    player->heading = spawn_point.heading;
    player->pitch = spawn_point.pitch;
    player->roll = spawn_point.roll;
}

void init_game(Sound_Output *sound_output, uint32 num_samples) {
    assert(!game_state->is_initted);
    game_state->mode = Game_Mode::EDITING;
    
    Arena_Allocator *game_data_arena = &memory.game_data;
    
    // init fps counters
    real64 current_time = platform_get_wall_clock_time();
    game_state->last_update_time = current_time;
    game_state->last_fps_update_time = current_time;
    
    Display_Output *display_output = &game_state->render_state.display_output;

    // init player
    init_player();

    // music file testing
#if 0
    File_Data music_file_data = platform_open_and_read_file((Allocator *) game_data_arena,
                                                            "../drive my car.wav");
    Wav_Data *wav_data = (Wav_Data *) music_file_data.contents;

    Audio_Source *music = &game_state->music;
    *music = make_audio_source(wav_data->subchunk_2_size / (wav_data->bits_per_sample / 8 * 2),
                               0, 1.0f, true, (int16 *) &wav_data->data);
#endif

    // init message manager
    game_state->message_manager = {};
    game_state->message_manager.message_time_limit = MESSAGE_TIME_LIMIT;
    message_manager = &game_state->message_manager;

    // init asset manager
    game_state->asset_manager = {};
    asset_manager = &game_state->asset_manager;
    init_asset_manager(&memory.game_data);
    load_default_assets();

    // init level
    init_level(&game_state->level, &memory.game_data);
    
    // load default level
    read_and_load_level(&game_state->level, "levels/beach-night.level");
    
    // init camera
    init_camera(&game_state->camera, display_output, CAMERA_FOV);

    Vec3 origin = make_vec3(0.0f, 10.0f, 0.0f);
    Vec3 direction = make_vec3(0.0f, -1.0f, 0.0f);
    Ray ray = make_ray(origin, direction);
    Transform transform = make_transform();
    transform.scale = make_vec3(1.0f, 0.5f, 1.0f);

    // init ui state
    ui_manager = &game_state->ui_manager;
    ui_init(&memory.ui_arena);

    // init editor
    init_editor(&memory.editor_arena, &game_state->editor_state, *display_output);
    
    game_state->is_initted = true;

    #if 0
    Normal_Entity test_entity = {};
    test_entity.mesh_name = make_string("mesh");
    test_entity.material_name = make_string("material");

    Marker m = begin_region();
    Buffer serialized_entity = serialize(temp_region, test_entity);
    end_region(m);

    add_mesh("ground", "blender/ground3.mesh");
    Mesh *car_mesh = add_mesh(Mesh_Type::LEVEL, "car", "blender/car.mesh");
    Mesh *ground_mesh = get_mesh("ground");
    delete_mesh("car");
    clear_level_meshes();
    #endif
}

#if 0
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
#endif

#if 0
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
#endif

inline void get_transformed_triangle(Mesh *mesh, int32 triangle_index, Mat4 *object_to_world,
                                     Vec3 transformed_triangle[3]) {
    get_triangle(mesh, triangle_index, transformed_triangle);
    transform_triangle(transformed_triangle, object_to_world);
}

bool32 move_player_to_closest_ground(Player *player, real32 max_drop_distance) {
    Level *level = &game_state->level;

    // NOTE: since we're using a ray to find the ground, it is possible that we can get stuck if our level
    //       has some type of chasm that doesn't fit the player, but has ground that was close enough that
    //       we moved the player to the bottom of the chasm. just don't put that type of geometry in levels.
    
    Ray ray = make_ray(player->position, -y_axis);
    real32 t_min = FLT_MAX;
    bool32 found_ground = false;

    {
        Entity *current = level->entities;
        while (current) {
            if (current->flags & ENTITY_MESH) {
                Mat4 object_to_world = get_model_matrix(current->transform);
                Mesh *mesh = get_mesh(current->mesh_id);

                // TODO: check against AABB first?
                Ray_Intersects_Mesh_Result result;
                if (ray_intersects_mesh(ray, mesh, current->transform, false, &result)) {
                    if (result.t < t_min && result.t <= max_drop_distance) {
                        t_min = result.t;
                        found_ground = true;
                    }
                }
            }
            
            current = current->next;
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

void do_collisions(Collision_Debug_Frame *debug_frame, Player *player, Vec3 new_pos) {
    Level *level = &game_state->level;

    real32 player_radius = 0.5f;
    bool32 found_ground = false;
    real32 min_ground_distance = INFINITY;
    Vec3 min_ground_normal = {};

    player->position = new_pos;
    Capsule current_capsule = make_capsule(player->position,
                                           player->position + make_vec3(0.0f, player->height, 0.0f),
                                           Player_Constants::capsule_radius);

    const int32 MAX_COLLISIONS = 128;
    Collision collisions[MAX_COLLISIONS];
    int32 num_collisions = 0;

    for (Entity *entity = level->entities; entity; entity = entity->next) {
        bool32 can_collide = (entity->flags & ENTITY_MESH) && (entity->flags & ENTITY_COLLIDER);
        if (!can_collide) {
            continue;
        }

        Mat4 object_to_world = get_model_matrix(entity->transform);
        Mesh *mesh = get_mesh(entity->mesh_id);

        for (int32 triangle_index = 0; triangle_index < (int32) mesh->num_triangles; triangle_index++) {
            Vec3 triangle[3];
            get_transformed_triangle(mesh, triangle_index, &object_to_world, triangle);

            Vec3 triangle_normal = get_triangle_normal(triangle);
            assert(!is_zero(triangle_normal));
            Vec3 penetration_normal, penetration_point;
            real32 penetration_depth, distance_from_plane;

            bool32 intersected = capsule_intersects_triangle(current_capsule, triangle,
                                                             &penetration_normal,
                                                             &penetration_depth,
                                                             &penetration_point,
                                                             &distance_from_plane);

            if (!intersected)
                continue;

            // TODO: we might be able to check this earlier
            if (distance_from_plane < 0.0f)
                continue;

            // we're colliding
            if (hit_bottom_player_capsule_sphere(&current_capsule, &penetration_point)) {
                real32 abs_normal_y = fabsf(triangle_normal.y);
                real32 y_at_45_deg = 0.707106781f;
                if (abs_normal_y > y_at_45_deg) {
                    found_ground = true;
                    // find the closest ground.. i.e. the one that is penetrating the deepest
                    real32 ground_distance = current_capsule.radius - penetration_depth;
                    if (ground_distance < min_ground_distance) {
                        min_ground_distance = ground_distance;
                        min_ground_normal = triangle_normal;

                        player->walk_state.triangle_normal = min_ground_normal;
                    }
                }
            }

            // TODO: we get pushed out on the same frame.
            // - can't we just check if there's anything below us, and if there isn't, then just bring us down to the closest ground
            // - just find the closest ground and move us to it
            // - i don't think the distance really matters because if it was on the same frame, then, actually idk
            //   - does this mess up jumping?
            /*
              - if you were grounded and are no longer grounded (this should be checked after we do_collision)
                - then it means you're in the air for whatever reason
                - which could be because:
                  - you're jumping
                  - you walked off a ledge
                  - you got pushed up during collision
                - jumping - easy to detect; just set a bool
                - walked off a ledge - idk
                - pushed up during collision:
                  - were grounded, but no longer grounded
                  - can we just check if the new y is higher?
                    - if you're on an upward slope that ends with a ledge, you can be not grounded after being grounded
                      and still have a new y that is higher
                    - i think it's fine to just do a closest ground thing; we just have to only do it within a certain
                      distance
                - can we use a cylinder triangle test?
                - go from bottom of capsule to set distance
                  - you can get the closest point to the cylinder ray
                  - then just move the player y down to the y of the intersection point
                    - intersection point meaning same as with capsule intersection
             */
            
            //player->position += triangle_normal*(current_capsule.radius - distance_from_plane);

            assert(num_collisions < MAX_COLLISIONS);
            Vec3 push_out = penetration_normal * penetration_depth;
            collisions[num_collisions++] = { current_capsule.radius - penetration_depth, push_out };

#if 0
            player->position += penetration_normal * penetration_depth;
            current_capsule = make_capsule(player->position,
                                           player->position + make_vec3(0.0f, player->height, 0.0f),
                                           Player_Constants::capsule_radius);
#endif
        }
    }

    // define a push out threshold, since we want to be somewhat inside the mesh
    // to prevent constantly switching between is_grounded=false and true
    #define PUSH_OUT_THRESHOLD 0.00001f
    if (num_collisions) {
        real32 min_distance = INFINITY;
        Collision *closest_collision = NULL;
        int32 min_dist_index = -1;
        for (int32 i = 0; i < num_collisions; i++) {
            Collision *c = &collisions[i];
            if ((c->distance_from_center < min_distance) && (distance(c->push_out) > 0.00001f)) {
                closest_collision = c;
                min_distance = c->distance_from_center;
            }
        }

        // TODO: we only push out on a single collision..
        // - may want to push out on more, so we don't get jerky collisions
        if (closest_collision) {
            player->position += closest_collision->push_out;
        }
    } else {
        // TODO: just move directly down if we found a point close enough
        player->position = new_pos;
    }
        
    Collision_Debug_Subframe position_subframe = {
        COLLISION_SUBFRAME_POSITION
    };
    position_subframe.position = {
        player->position
    };
    collision_debug_log_subframe(debug_frame, position_subframe);

    player->is_grounded = found_ground;
}

void do_collisions_point(Collision_Debug_Frame *debug_frame, Player *player, Vec3 new_pos) {
    Level *level = &game_state->level;

    real32 player_radius = 0.5f;
    bool32 has_collision = false;
    
    for (Entity *entity = level->entities; entity; entity = entity->next) {
        bool32 can_collide = (entity->flags & ENTITY_MESH) && (entity->flags & ENTITY_COLLIDER);
        if (!can_collide) {
            continue;
        }

        Mat4 object_to_world = get_model_matrix(entity->transform);
        Mesh *mesh = get_mesh(entity->mesh_id);

        for (int32 triangle_index = 0; triangle_index < (int32) mesh->num_triangles; triangle_index++) {
            Vec3 triangle[3];
            get_transformed_triangle(mesh, triangle_index, &object_to_world, triangle);

            Vec3 triangle_normal = get_triangle_normal(triangle);

            bool32 isWall = triangle_normal.y > -0.5f && triangle_normal.y < 0.5f;
            if (!isWall){
                continue;
            }

            // project the new position onto the normal
            real32 distance_from_triangle_plane = dot(new_pos - triangle[0], triangle_normal);
            if (distance_from_triangle_plane < 0.0f) {
                // behind the wall, it's fine
                // TODO: should probably shoot a ray and see if we collide, honestly
                continue;
            }
            if (distance_from_triangle_plane > player_radius) {
                // not colliding within player radius
                continue;
            }

            // we're within the collision radius/distance of the triangle.
            // now, check if the new position (projected onto the triangle) is inside the triangle.

            // get the closest point on the triangle plane to the point. NEVERMIND, don't need to do this,
            // since our barycentric coordinate function just projects the point onto the triangle.

            // check if that coplanar point exists in the triangle.
            Vec3 bary_coords;
            if (!compute_barycentric_coords(triangle[0], triangle[1], triangle[2],
                                            triangle_normal, new_pos,
                                            &bary_coords)) {
                assert(!"Degenerate triangle!");
                continue;
            }

            // check if bary coords are inside the triangle
            real32 bary_coords_sum = bary_coords[0] + bary_coords[1] + bary_coords[2];
            if (bary_coords[0] < 0.0f || bary_coords[1] < 0.0f || bary_coords[2] < 0.0f ||
                bary_coords_sum > 1.0f) {
                continue;
            }

            // we're colliding.
            real32 push_out_distance = 0.5f - distance_from_triangle_plane;
            Vec3 push_out_vec = triangle_normal * push_out_distance;

            // TODO: we shouldn't do this here. instead, we should just save all the push_out_vecs, average
            // them, then do the same calculation as below.
            player->position = new_pos + push_out_vec;
            has_collision = true;
        }
    }

    if (!has_collision) {
        player->position = new_pos;
    }
}

void old_do_collisions2(Collision_Debug_Frame *debug_frame, Player *player, Vec3 initial_move) {
    Level *level = &game_state->level;

    bool32 found_ground = false;
    bool32 intersected = false;

    // we use the initial_move vector here, since using the correction displacement from a previous
    // intersection can result in behaviour such as pushing the player through meshes
    // (you can see this if you try pushing yourself underneath a sloped plane made up of 2
    // triangles)
    Vec3 displacement = initial_move;
    real32 displacement_length = distance(displacement);
    assert(displacement_length > EPSILON);
    player->position += displacement;

    Capsule player_capsule = make_capsule(player->position,
                                          player->position + make_vec3(0.0f, player->height, 0.0f),
                                          Player_Constants::capsule_radius);
    
    for (int32 num_collisions = 0; num_collisions < MAX_FRAME_COLLISIONS; num_collisions++) {
        Entity *entity = level->entities;

        while (entity) {
            bool32 walkable = (entity->flags & ENTITY_MESH) && (entity->flags & ENTITY_COLLIDER);
            if (!walkable) {
                entity = entity->next;
                continue;
            }

            Mat4 object_to_world = get_model_matrix(entity->transform);
            Mesh *mesh = get_mesh(entity->mesh_id);

            for (int32 triangle_index = 0; triangle_index < (int32) mesh->num_triangles; triangle_index++) {
                Vec3 triangle[3];
                get_transformed_triangle(mesh, triangle_index, &object_to_world, triangle);

                Vec3 triangle_normal = get_triangle_normal(triangle);

                Vec3 penetration_normal, penetration_point;
                real32 penetration_depth, distance_from_plane;

                intersected = capsule_intersects_triangle(player_capsule, triangle,
                                                          &penetration_normal,
                                                          &penetration_depth,
                                                          &penetration_point,
                                                          &distance_from_plane);

                // TODO: there is a bug in this code when sliding under and along two adjacent triangles above you.
                //       since the adjacent triangles are not being treated as a single plane, you can slide along
                //       one, but then get caught on the other when you hit it. if the second triangle were the only
                //       one there, this would be correct behaviour.
                if (intersected) {
                    Vec3 normalized_displacement = displacement / displacement_length;

                    Vec3 initial_position = player->position;

                    Vec3 correction = penetration_normal * (penetration_depth + 0.00001f);
                    player->position += correction;

                    // we check this so that we can nicely climb up triangles that are low enough without
                    // being pushed brought down by gravity. this is also necessary since if we didn't have this,
                    // if a wall triangle comes before a floor triangle, then we would get pushed out before
                    // we'd even be able to intersect with the floor and walk on the floor.
#if 1
                    if (hit_bottom_player_capsule_sphere(&player_capsule, &penetration_point)) {
                        found_ground = true;
                        player->walk_state.triangle_normal = triangle_normal;
                    }
#endif

                    // TODO: we don't save the correction vector yet
                    // this is the capsule that we're doing the collision test on
                    Collision_Debug_Subframe collision_subframe = {
                        COLLISION_SUBFRAME_COLLISION
                    };
                    collision_subframe.collision = {
                        initial_position,
                        entity->id,
                        triangle_index,
                        triangle_normal,
                        penetration_normal,
                        penetration_depth,
                        penetration_point
                    };
                    collision_debug_log_subframe(debug_frame, collision_subframe);

                    // this is the new, corrected, position
                    Collision_Debug_Subframe position_subframe = {
                        COLLISION_SUBFRAME_POSITION
                    };
                    position_subframe.position = {
                        player->position
                    };
                    collision_debug_log_subframe(debug_frame, position_subframe);
                    
                    displacement = correction;

#if 0
                    Collision_Debug_Subframe desired_move_subframe = { COLLISION_SUBFRAME_DESIRED_MOVE };
                    desired_move_subframe.desired_move = { player->position, displacement };
                    collision_debug_log_subframe(debug_frame, desired_move_subframe);
#endif
                        
                    player_capsule = make_capsule(player->position,
                                                  player->position + make_vec3(0.0f, player->height, 0.0f),
                                                  Player_Constants::capsule_radius);
                        
                    //real32 displacement_length = distance(initial_move);

                    // we don't early-out here since we might encounter a walkable surface later, on this same
                    // mesh.
                    //break;

#if DEBUG_SHOW_COLLISION_LINES
                    Vec4 triangle_color = make_vec4(1.0f, 1.0f, 1.0f, 1.0f);
                    add_debug_line(&game_state->debug_state,
                                   triangle[0], triangle[1], triangle_color);
                    add_debug_line(&game_state->debug_state,
                                   triangle[1], triangle[2], triangle_color);
                    add_debug_line(&game_state->debug_state,
                                   triangle[2], triangle[0], triangle_color);
#endif

                    break;
                }
            }

            // TODO: not sure if we should keep this early-out
            if (intersected) break;
                
            entity = entity->next;
        }
        
        if (!intersected) break;
    }

    // can we just not do gravity checks? idk
    // is_grounded should only be set to false after a move
    // if you're pushed out by the ground, i think that guarantees you're still grounded right?
    bool32 was_grounded = player->is_grounded && !found_ground;
    
    player->is_grounded = found_ground;
}

void old_do_collisions(Collision_Debug_Frame *debug_frame, Player *player, Vec3 initial_move) {
    Level *level = &game_state->level;

    bool32 found_ground = false;
    bool32 intersected = false;

    // we use the initial_move vector here, since using the correction displacement from a previous
    // intersection can result in behaviour such as pushing the player through meshes
    // (you can see this if you try pushing yourself underneath a sloped plane made up of 2
    // triangles)
    Vec3 displacement = initial_move;
    real32 displacement_length = distance(displacement);
    assert(displacement_length > EPSILON);
    player->position += displacement;

    Capsule player_capsule = make_capsule(player->position,
                                          player->position + make_vec3(0.0f, player->height, 0.0f),
                                          Player_Constants::capsule_radius);
    
    for (int32 num_collisions = 0; num_collisions < MAX_FRAME_COLLISIONS; num_collisions++) {
        Entity *entity = level->entities;

        while (entity) {
            bool32 walkable = (entity->flags & ENTITY_MESH) && (entity->flags & ENTITY_COLLIDER);
            if (!walkable) {
                entity = entity->next;
                continue;
            }

            Mat4 object_to_world = get_model_matrix(entity->transform);
            Mesh *mesh = get_mesh(entity->mesh_id);

            for (int32 triangle_index = 0; triangle_index < (int32) mesh->num_triangles; triangle_index++) {
                Vec3 triangle[3];
                get_transformed_triangle(mesh, triangle_index, &object_to_world, triangle);

                Vec3 triangle_normal = get_triangle_normal(triangle);

                Vec3 penetration_normal, penetration_point;
                real32 penetration_depth, distance_from_plane;

                intersected = capsule_intersects_triangle(player_capsule, triangle,
                                                          &penetration_normal,
                                                          &penetration_depth,
                                                          &penetration_point,
                                                          &distance_from_plane);

                // TODO: there is a bug in this code when sliding under and along two adjacent triangles above you.
                //       since the adjacent triangles are not being treated as a single plane, you can slide along
                //       one, but then get caught on the other when you hit it. if the second triangle were the only
                //       one there, this would be correct behaviour.
                if (intersected) {
                    // TODO: add the intersection to the frame's intersection array
                    //       - the intersection is in order
                    //       - each struct contains data for the triangle and the resolution vectors
#if 1
#if 0
                    if (distance(displacement) < EPSILON) {
                        Vec3 correction = -penetration_normal * (penetration_depth + 0.00001f);
                        player->position += correction;

                        if (hit_bottom_player_capsule_sphere(&player_capsule, &penetration_point)) {
                            found_ground = true;
                            player->walk_state.triangle_normal = triangle_normal;
                        }

                        player_capsule = make_capsule(player->position,
                                                      player->position + make_vec3(0.0f, player->height, 0.0f),
                                                      Player_Constants::capsule_radius);

                        // TODO: set the displacement and displacement_length to the correction vector
                        displacement = correction;
                    } else if (dot(displacement, triangle_normal) < 0.0f) {
#endif
                    // we don't want to do anything when it's like 90 degrees right?
                    //if (dot(displacement, triangle_normal) < 0.0f) {
                    if (dot(displacement, triangle_normal) < 0.0f &&
                        fabsf(dot(displacement, triangle_normal)) > EPSILON) {
                        // TODO: we need to replace this don't we?
                        Vec3 normalized_displacement = displacement / displacement_length;

                        // TODO: wtf is this? vvvvv
                        // i'm pretty sure penetration_normal points towards the center of the capsule cap sphere
                        // TODO: i think if we're moving 1 unit, the player should move 1 unit up the slope if they collide
                        // - is this currently the case? i'm not sure if this calculation ensures that happens

                        // the displacement is wrong...???
                        // TODO: i don't think this is even normalized?
                        //       - why would we even do this? what's the point of multiplying
                        //         displacement_length by it?
                        //       - what if we just do displacement instead of normalized displacement to dot?
#if 0
                        Vec3 normalized_correction = (-penetration_normal *
                                                      dot(normalized_displacement, penetration_normal));
#else
                        #if 0
                        Vec3 correction = (-penetration_normal *
                                           dot(displacement, penetration_normal));
                        if (distance(displacement) < EPSILON) {
                            correction = penetration_normal * (penetration_depth + 0.00001f);
                        }
                        #else
                        Vec3 correction = penetration_normal * (penetration_depth + 0.00001f);
                        #endif
#endif

                        // TODO: this causes jittering when pushing into things, but we want to do it this way
                        //       since just pushing out by the penetration depth won't give you the correct speed
                        //       when sliding along slopes or walls. also, this causes us to move out too much when
                        //       falling into the ground. we get pushed out of the ground, then we attempt to
                        //       collide with the ground again with gravity so that we don't keep alternating
                        //       is_grounded, but with this uncommented, we get pushed out so much that the
                        //       displacement from gravity or some small displacement we use to push us downwards
                        //       isn't enough for us to collide again with the ground, and so we get the
                        //       undesirable behaviour of is_grounded constantly alternating while walking on a
                        //       surface.
                        //       also i think this (the commented out line) is really buggy. you can get stuck
                        //       at the common vertex of  multiple triangles in a mesh.

                        Vec3 initial_position = player->position;
                        
                        #if 1
                        //player->position += displacement_length * normalized_correction; // line before
                        player->position += correction;
                        #if 0
                        if (fabsf(dot(normalized_displacement, penetration_normal)) < (1 - EPSILON)) {
                            player->position += displacement_length * normalized_correction; // line before
                        } else {
                            player->position += penetration_normal * (penetration_depth + 0.00001f);
                            //player->position += displacement_length * normalized_displacement;
                        }
                        #endif
                        #endif
                        //player->position += penetration_normal * (penetration_depth + 0.00001f);

                        // we check this so that we can nicely climb up triangles that are low enough without
                        // being pushed brought down by gravity. this is also necessary since if we didn't have this,
                        // if a wall triangle comes before a floor triangle, then we would get pushed out before
                        // we'd even be able to intersect with the floor and walk on the floor.
                        if (hit_bottom_player_capsule_sphere(&player_capsule, &penetration_point)) {
                            found_ground = true;
                            player->walk_state.triangle_normal = triangle_normal;
                        }

                        // TODO: we don't save the correction vector yet
                        Collision_Debug_Subframe collision_subframe = {
                            COLLISION_SUBFRAME_DESIRED_MOVE_COLLISION
                        };
                        collision_subframe.desired_move_collision = {
                            initial_position,
                            displacement,
                            entity->id,
                            triangle_index,
                            triangle_normal,
                            penetration_normal,
                            penetration_depth,
                            penetration_point
                        };
                        collision_debug_log_subframe(debug_frame, collision_subframe);
                        
#if 0
                        Collision_Debug_Intersection info = {
                            initial_position,
                            player->position,
                            displacement,
                            entity->id,
                            triangle_index,
                            triangle_normal,
                            penetration_normal,
                            penetration_depth,
                            penetration_point
                        };
                        collision_debug_add_intersection(debug_frame, info);
#endif
                        
                        displacement = correction;

                        Collision_Debug_Subframe desired_move_subframe = { COLLISION_SUBFRAME_DESIRED_MOVE };
                        desired_move_subframe.desired_move = { player->position, displacement };
                        collision_debug_log_subframe(debug_frame, desired_move_subframe);
                        
                        player_capsule = make_capsule(player->position + displacement,
                                                      player->position + make_vec3(0.0f, player->height, 0.0f),
                                                      Player_Constants::capsule_radius);
                        
                        //real32 displacement_length = distance(initial_move);

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
                
            entity = entity->next;
        }
        
        if (!intersected) break;
    }

    // can we just not do gravity checks? idk
    // is_grounded should only be set to false after a move
    // if you're pushed out by the ground, i think that guarantees you're still grounded right?
    bool32 was_grounded = player->is_grounded && !found_ground;
    
    player->is_grounded = found_ground;
}

void update_player(Player *player ,Controller_State *controller_state, real32 dt) {
    Debug_State *debug_state = &game_state->debug_state;

    // cap dt.. just to reduce issues
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

#if 0
    if (player->is_grounded) {
        // make basis for walking on triangle.
        // this makes it so we can actually set the speeds ourselves for when we're traveling
        // up or down slopes and not just be based on either the push out vector or the "go down"
        // vector (the vector that we use to push the player down when going down shallow slopes
        // to prevent air time due to a move vector that isn't along the triangle's plane).
        // right now we're always just going at the default player speed whether we're going
        // up or down a slope. without this, going up slopes would be slow and going down slopes
        // would be fast (compared to walking on flat surface).
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
#endif

    bool32 is_wasd_move = false;
    Vec3 move_vector = make_vec3();
    if (controller_state->key_w.is_down) {
        //move_vector = dot(heading_direction, player_direction) * heading_direction;
        move_vector += player_forward;
        is_wasd_move = true;
    }
    if (controller_state->key_s.is_down) {
        move_vector += -player_forward;
        is_wasd_move = true;
    }
    if (controller_state->key_d.is_down) {
        move_vector += player_right;
        is_wasd_move = true;
    }
    if (controller_state->key_a.is_down) {
        move_vector += -player_right;
        is_wasd_move = true;
    }
    if (controller_state->key_space.is_down) {
        if (player->is_grounded) {
            // TODO: i'm not sure why we have to have such high values for y velocity... we might be doing something wrong?
            player->velocity = make_vec3(0.0f, 5.0f, 0.0f);
        }
        is_wasd_move = false;
    }
    if (controller_state->key_shift.is_down) {
        move_vector += -y_axis;
    }
    move_vector = normalize(move_vector) * player->speed * dt;

    Vec3 gravity_acceleration = make_vec3(0.0f, -9.81f, 0.0f);

    Collision_Debug_State *collision_debug_state = &game_state->editor_state.collision_debug_state;
    Collision_Debug_Frame *collision_debug_frame = collision_debug_start_frame(collision_debug_state,
                                                                               player->position);

    collision_debug_log_position_subframe(collision_debug_frame, player->position);
    
    Vec3 displacement_vector = player->velocity*dt + 0.5f*player->acceleration*dt*dt;
    player->velocity += player->acceleration * dt;

    move_vector += displacement_vector;
    
    bool32 was_grounded = player->is_grounded;
    Vec3 old_position = player->position;
    
    do_collisions(collision_debug_frame, player, player->position + move_vector);

    if (!player->is_grounded && was_grounded && is_wasd_move) {
        // try moving the player down. this should only be for when we're
        // walking down a shallow slope to prevent air-time. the conditions
        // for this is to prevent weird behaviour where we snap to the ground
        // when when we're falling down and we're almost at the ground.
        Vec3 go_down_vector = make_vec3(0.0f, -5.0f * dt, 0.0f);
        Vec3 test_position = player->position + go_down_vector;
        do_collisions(collision_debug_frame, player, player->position + go_down_vector);

        Vec3 push_out_delta = player->position - test_position;
        Vec3 push_out_dir = normalize(push_out_delta);
        // only if we really pushed out a significant amount
        // TODO: do we always want to do this? error might accumulate if we don't always do this..
        if (push_out_dir.y > EPSILON && fabsf(distance(push_out_delta)) > EPSILON) {
            // TODO: debug this.. we aren't hitting a breakpoint here

            // check if the delta is very close to being straight up
            // TODO: test what happens when we're pushed out downwards, but that
            //       shouldn't happen because this is for walking on the ground..
            bool32 is_delta_not_straight_up = fabsf(push_out_dir.y - 1.0f) > EPSILON;
            if (is_delta_not_straight_up) {
                // if it's not, then we need to make it straight down
                // i.e. we shouldn't push out along the push out vector, but straight up
                // such that we're still pushed out of the geometry, but the new position
                // is straight above the old position (the position we passed to do_collisions())

                // make a basis from the push out vector
                Vec3 push_out_right, push_out_up;
                make_basis(push_out_dir, &push_out_right, &push_out_up);

                Vec3 b1 = push_out_right;
                Vec3 b2 = push_out_up;

                Vec3 p = push_out_delta;

                if (fabsf(b1.x) < 0.0001f) {
                    // swap to avoid divide by zero
                    Vec3 temp = b1;
                    b1 = b2;
                    b2 = temp;
                }
                
                assert(fabsf(b1.x) > 0.0001f);
                assert(fabsf(-b1.z*b2.x/b1.x + b2.z) > 0.00001f);

                real32 t2 = ( -p.z + (p.x*b1.z) / b1.x ) / ( (-b2.x*b1.z)/b1.x + b2.z );
                real32 t1 = (-t2*b2.x - p.x) / b1.x;

                real32 new_y = test_position.y + t1*b1.y + t2*b2.y + p.y;
                
                player->position = { test_position.x, new_y, test_position.z };
            }
        }

        // (x, y, z) + (p.x, p.y, p.z) = (x', y', z')
        // (x_2, y_2, z_2) + (p.x, p.y, p.z) = (x, y', z)

        // we can just push out, then figure out the x and y that makes the
        // push out vector the same. we know x since it's just the difference.

        // (x, y) + (p.x, p.y) = (x', y')
        // (x_2, y_2) + (p.x, p.y) = (x, y_2')
        // (x, y_2) + (p.x, p.y) = (x, y_2')

        // (x, y) + (a, b) + (p.x, p.y) = (x, y')
        // x = x + a + p.x
        // -> a = -p.x

        // (x, y) 
        
        // y' = y + b + p.y

        // (x, y) + (p.x, p.y) = (x', y')
        // (x - a, y) + (p.x, p.y) = (x, y_2)

        // (x, y) + t*(vec orthogonal to p) + (p.x, p.y) = (x, y_2)
        // x + t*ortho_p.x + p.x = x
        // -> t*ortho_p.x + p.x = 0
        // -> t = -p.x / ortho_p.x

        // y + t*ortho_p.y + p.y = y_2
        // we know all of these..

        // NOTE: this is how this works..
        // - get basis vectors for the push out vector
        //   - basically coordinate space for the triangle's plane
        // - figure out linear combination of the basis vectors such that the point
        //   created from the linear combination + the push out vector (p) = (x, y', z)
        // - where x, z are the x, z of test_position (the pushed down position of the player, i.e.
        //   the position we call do_collisions() with)
        // - but y' is the y where the capsule is no longer colliding with any geometry, but is still
        //   straight above the test_position

        // (x, y, z) + t1*b1 + t2*b2 + (p.x, p.y, p.z) = (x, y', z)
        // (x, y, z) + (t1*b1.x, t1*b1.y, t1*b1.z) + (t2*b2.x, t2*b2.y, t2*b2.z) + (p.x, p.y, p.z) = (x, y', z)

        // TODO: test the solution out...

        // three equations:
        // x + t1*b1.x + t2*b2.x + p.x = x
        // y + t1*b1.y + t2*b2.y + p.y = y'
        // z + t1*b1.z + t2*b2.z + p.z = z

        // unknowns: t1, t2, y'

        // t1*b1.x + t2*b2.x + p.x = 0
        // t1*b1.z + t2*b2.z + p.z = 0

        //pg 191
        
        /*
          isolate t1
          t1 = (-t2*b2.x - p.x) / b1.x
          
          sub into other equations:
          1. y + ((-t2*b2.x - p.x) / b1.x)*b1.y + t2*b2.y + p.y = y'
          2. ((-t2*b2.x - p.x) / b1.x)*b1.z + t2*b2.z + p.z = 0

          isolate t2 in eqn. 2
          ((-t2*b2.x - p.x) / b1.x)*b1.z + t2*b2.z + p.z = 0
          (-t2*b2.x*b1.z - p.x*b1.z) / b1.x + t2*b2.z + p.z = 0

          ditsribute b1.x
          (-t2*b2.x*b1.z) / b1.x - (p.x*b1.z) / b1.x + t2*b2.z = -p.z
          (-t2*b2.x*b1.z) / b1.x + t2*b2.z = -p.z + (p.x*b1.z) / b1.x

          factor out t2
          t2 * [ (-b2.x*b1.z)/b1.x + b2.z ] = -p.z + (p.x*b1.z) / b1.x
          t2 = [-p.z + (p.x*b1.z) / b1.x ] / [ (-b2.x*b1.z)/b1.x + b2.z ]
          t2 = (-p.z + p.x*b1.z) / ((-b2.x*b1.z) + b2.z*b1.x)




          t2 * [ (-b2.x*b1.z)/b1.x + b2.z ] = -p.z
          t2 = -p.z / [ (-b2.x*b1.z)/b1.x + b2.z ]


          


         */

        
        // t1 = (-t2*b2.x - p.x) / b1.x

        // y + ((-t2*b2.x - p.x) / b1.x)*b1.y + t2*b2.y + p.y = y'
        // z + ((-t2*b2.x - p.x) / b1.x)*b1.z + t2*b2.z + p.z = z

        // unknowns: t2, y'
        // z + ((-t2*b2.x - p.x) / b1.x)*b1.z + t2*b2.z + p.z = z

        // rearrange to get t2:
        // ((-t2*b2.x - p.x) / b1.x)*b1.z + t2*b2.z + p.z = 0
        // (b1.z/b1.x) * (-t2*b2.x - p.x) + t2*b2.z + p.z = 0
        // (-b1.z*t2*b2.x - b1.z*p.x)/b1.x + t2*b2.z + p.z = 0
        // t2*(-b1.z*b2.x/b1.x) - b1.z*p.x/b1.x + t2*b2.z + p.z = 0
        // t2 + t2*b2.z = b1.z*p.x/b1.x - p.z
        // t2 * (-b1.z*b2.x/b1.x + b2.z) = b1.z*p.x/b1.x - p.z
        // t2 = (b1.z*p.x/b1.x - p.z) / (-b1.z*b2.x/b1.x + b2.z)

        // unknown: y'
        // y + t1*b1.y + t2*b2.y + p.y = y'

        // push out vector should be the same
        
        // we want x and z to be the same as before.
        // we need a y such that the push out vector is the same distance.

        // it's not even true that it'll still be on the same triangle if we
        // move the player up instead of along the penetration vector..
        // it doesn't matter, i think.

        // it is the same distance out.. even if we move up.
        // so if we just push out, then move along the triangle such that we
        // end up in the same spot, that should be basically the same thing
    }

    // TODO: fix speed up or slow down cause by going down or up slopes
    // - this currently caps other collisions.. and also movement due to gravity
    // - if we were grounded, are still grounded, and the push out vector was due to the ground,
    //   then fix this delta..
    Vec3 delta = player->position - old_position;
    if (distance(delta)-player->speed*dt > EPSILON) {
        player->position = old_position + normalize(delta)*player->speed*dt;
    }

    char *dist_text = string_format((Allocator *) &ui_manager->frame_arena, "distance travelled: %f",
                                    distance(delta));
    draw_debug_text(make_vec2(5.0f, 50.0f), dist_text);

    char *expected_dist_text = string_format((Allocator *) &ui_manager->frame_arena,
                                             "expected distance: %f",
                                             player->speed * dt);
    draw_debug_text(make_vec2(5.0f, 65.0f), expected_dist_text);

    if (player->is_grounded) {
        player->velocity = {};
        player->acceleration = {};
    } else {
        player->acceleration = gravity_acceleration;
    }
    
    #if 0
    if (player->is_grounded) {
        player->velocity = move_vector;
        player->acceleration = {};
        Vec3 displacement_vector = player->velocity*dt;

        bool32 initial_is_grounded = player->is_grounded;

        if (distance(displacement_vector) > EPSILON) {
            // TODO: rename do_collisions - do_collisions actually does the move given by displacement_vector

            bool32 is_player_move = distance(move_vector) > EPSILON;
            Collision_Debug_Subframe desired_move_subframe = { COLLISION_SUBFRAME_DESIRED_MOVE };
            desired_move_subframe.desired_move = { player->position, displacement_vector, is_player_move };
            collision_debug_log_subframe(collision_debug_frame, desired_move_subframe);
            
            do_collisions(collision_debug_frame, player, displacement_vector);
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

                desired_move_subframe = { COLLISION_SUBFRAME_DESIRED_MOVE };
                desired_move_subframe.desired_move = { player->position, displacement_vector, false };
                collision_debug_log_subframe(collision_debug_frame, desired_move_subframe);
                
                do_collisions(collision_debug_frame, player, displacement_vector);
            }
        }

        bool32 was_grounded = initial_is_grounded && !player->is_grounded;

        if (was_grounded) {
            // TODO: should we log some collision debug info here?
            Vec3 before_drop_move_position = player->position;
            bool32 moved = move_player_to_closest_ground(player, MAX_DROP_DISTANCE);
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

        Collision_Debug_Subframe desired_move_subframe = { COLLISION_SUBFRAME_DESIRED_MOVE };
        desired_move_subframe.desired_move = { player->position, displacement_vector, false };
        collision_debug_log_subframe(collision_debug_frame, desired_move_subframe);
        
        if (distance(displacement_vector) > EPSILON) {
            do_collisions(collision_debug_frame, player, displacement_vector);
        }
    }
    #endif

    collision_debug_end_frame(collision_debug_state, collision_debug_frame, player->position);
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

void update_entities(Level *level, real32 dt) {
    Entity *current = level->entities;
    while (current) {
        // TODO: this is just for testing
        current->animation_t += dt;
        #if 0
        if (current->animation_id >= 0) {
            current->animation_t += dt;
        }
        #endif

        current = current->next;
    }
}

void update_game(Controller_State *controller_state, real32 dt) {
    assert(game_state->is_initted);
    
    Player *player = &game_state->player;
    update_entities(&game_state->level, dt);
    update_player(player, controller_state, dt);

    update_camera(&game_state->camera, &game_state->render_state.display_output,
                  player->position + make_vec3(0.0f, player->height, 0.0f),
                  player->heading, player->pitch, player->roll);
    update_render_state(game_state->camera);
}

void draw_debug_text(Vec2 position, char *text) {
    ui_push_existing_widget(ui_manager->always_on_top_layer);
    {
        UI_Theme white_text = NULL_THEME;
        white_text.text_color = { 1.0f, 1.0f, 1.0f, 1.0f };
        white_text.semantic_position = position;
    
        do_text(text, "", white_text);
    }
    ui_pop_widget();
}

void draw_ui(real32 dt) {
    // fps counter
    ui_push_existing_widget(ui_manager->always_on_top_layer);
    {
        UI_Theme white_text = NULL_THEME;
        white_text.text_color = { 1.0f, 1.0f, 1.0f, 1.0f };
        white_text.semantic_position = { 5.0f, 20.0f };
    
        char *fps_text = string_format((Allocator *) &ui_manager->frame_arena, "FPS: %d / dt %.3f",
                                       (int32) round(game_state->last_second_fps), dt);
        do_text(fps_text, "", white_text);

        UI_Theme is_grounded_theme = white_text;
        is_grounded_theme.semantic_position.y += 15.0f;

        char *grounded_text = string_format((Allocator *) &ui_manager->frame_arena, "is_grounded: %d",
                                            game_state->player.is_grounded);
        do_text(grounded_text, "", is_grounded_theme);
    }
    ui_pop_widget();
}

void update(Controller_State *controller_state,
            Sound_Output *sound_output, uint32 num_samples) {
    Display_Output *display_output = &game_state->render_state.display_output;
    if (!game_state->is_initted) {
        debug_print("initting game %d!\n", 123);
        init_game(sound_output, num_samples);
    }

    real64 current_time = platform_get_wall_clock_time();
    real32 dt = (real32) (current_time - game_state->last_update_time);
    game_state->last_update_time = current_time;
    game_state->dt = dt;

    ui_frame_init(dt);
    
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
            game_state->mode = Game_Mode::PLAYING;
            platform_set_cursor_visible(false);
        } else {
            game_state->mode = Game_Mode::EDITING;

            // set freecam to false; i feel like this is less disorienting if you switch to play mode
            // while in freecam mode then switch back to editing mode.
            game_state->editor_state.use_freecam = false;
            platform_set_cursor_visible(!game_state->editor_state.use_freecam);

            // debug
            #if 0
            FOR_LIST_NODES(Entity *, game_state->editor_state.level.entities) {
                Entity *entity = current_node->value;
                if (entity->type == ENTITY_NORMAL) {
                    Normal_Entity *e = (Normal_Entity *) entity;
                    if (e->collider.type == Collider_Type::CAPSULE) {
                        update_entity_position(&game_state->editor_state.asset_manager, entity, game_state->player.position);
                    }
                }
            }
            #endif
        }
    }

    ui_manager->last_frame_active = ui_manager->active;

    //update_render_state(render_state);

    // go through the asset update queues and update them
    update_assets_from_queues();
    
    if (game_state->mode == Game_Mode::EDITING) {
        update_editor(controller_state, dt);
        update_entities(&game_state->level, dt);
        draw_editor(Context::controller_state);
        game_state->editor_state.is_startup = false;
    } else {
        //asset_manager = &game_state->asset_manager;
        update_game(controller_state, dt);
    }

    draw_ui(dt);
    
    Player *player = &game_state->player;

    fill_sound_buffer_with_audio(sound_output, game_state->is_playing_music, &game_state->music, num_samples);

    update_messages(&game_state->message_manager, dt);
    draw_messages(&game_state->message_manager, display_output->height - MESSAGE_Y_OFFSET_FROM_BOTTOM);
    
    ui_post_update();

    if (game_state->mode == Game_Mode::EDITING) {
        // note that this won't run if you change modes from EDITING this frame
        editor_post_update();
    }
}
