#include "linked_list.h"
#include "editor.h"

UI_Button_Theme editor_button_theme = {
    { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE },
    { 0.0f, 20.0f }, { 0.0f, 0.0f },
    DEFAULT_BUTTON_BACKGROUND, DEFAULT_BUTTON_HOT_BACKGROUND, DEFAULT_BUTTON_ACTIVE_BACKGROUND,
    { 1.0f, 1.0f, 1.0f, 1.0f },
    default_font
};

UI_Button_Theme editor_button_danger_theme = {
    { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE },
    { 0.0f, 20.0f }, { 0.0f, 0.0f },
    DANGER_BUTTON_BACKGROUND, DANGER_BUTTON_HOT_BACKGROUND, DANGER_BUTTON_ACTIVE_BACKGROUND,
    { 1.0f, 1.0f, 1.0f, 1.0f },
    default_font
};

UI_Text_Field_Theme editor_text_field_theme = {
    editor_button_theme.background_color,
    editor_button_theme.hot_background_color,
    editor_button_theme.active_background_color,
    rgb_to_vec4(0, 255, 0),
    0.0f, 0, 0, { 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f,
    default_font,
    { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE },
    { 0.0f, editor_button_theme.size.y }
};

UI_Button_Theme editor_dropdown_item_theme = {
    editor_button_theme.size_type,
    editor_button_theme.size, editor_button_theme.position,
    rgb_to_vec4(19, 19, 23), rgb_to_vec4(36, 36, 43), rgb_to_vec4(8, 8, 10),
    editor_button_theme.text_color,
    editor_button_theme.font,
    UI_SCISSOR_INHERIT
};

UI_Button_Theme editor_selected_dropdown_item_theme = {
    editor_button_theme.size_type,
    editor_button_theme.size, editor_button_theme.position,
    rgb_to_vec4(61, 96, 252), rgb_to_vec4(61, 96, 252), rgb_to_vec4(61, 96, 252),
    editor_button_theme.text_color,
    editor_button_theme.font,
    UI_SCISSOR_INHERIT
};

UI_Text_Field_Slider_Theme editor_slider_theme = {
    DEFAULT_BUTTON_BACKGROUND,
    DEFAULT_BUTTON_HOT_BACKGROUND,
    DEFAULT_BUTTON_ACTIVE_BACKGROUND,
    
    rgb_to_vec4(61, 73, 60),
    rgb_to_vec4(0, 255, 0),
    true,

    0.0f, 0, 0, {}, 0.0f,

    default_font,

    { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE },
    { 0.0f, 20.0f }
};

UI_Checkbox_Theme editor_checkbox_theme = {
    DEFAULT_BUTTON_BACKGROUND,
    DEFAULT_BUTTON_HOT_BACKGROUND,
    DEFAULT_BUTTON_ACTIVE_BACKGROUND,

    rgb_to_vec4(255, 255, 255),
    { 20.0f, 20.0f }
};

void reset_asset_library_state() {
    Asset_Library_State *asset_library_state = &game_state->editor_state.asset_library_state;

    asset_library_state->material_modified = true;

#if 0
    asset_library_state->material_albedo_texture_modified = true;

    asset_library_state->material_metalness_modified = true;
    asset_library_state->material_metalness_texture_modified = true;

    asset_library_state->material_roughness_modified = true;
    asset_library_state->material_roughness_texture_modified = true;
#endif

    asset_library_state->selected_material_id = -1;
}

void init_editor(Arena_Allocator *editor_arena, Editor_State *editor_state, Display_Output display_output) {
    *editor_state = {};

    editor_state->arena = editor_arena;
    
    editor_state->selected_entity_id = -1;
    editor_state->last_selected_entity_id = -1;
    editor_state->show_wireframe = true;
    editor_state->show_colliders = true;
    editor_state->is_startup = true;

    reset_asset_library_state();
    
    init_camera(&editor_state->camera, &display_output, CAMERA_FOV);

    // init gizmo
    Gizmo_State *gizmo_state = &editor_state->gizmo_state;
    gizmo_state->arrow_mesh_name  = make_string("gizmo_arrow");
    gizmo_state->ring_mesh_name   = make_string("gizmo_ring");
    gizmo_state->sphere_mesh_name = make_string("gizmo_sphere");
    gizmo_state->cube_mesh_name   = make_string("gizmo_cube");
}

Entity *get_selected_entity(Editor_State *editor_state) {
    Level *level = &game_state->level;
    int32 selected_id = editor_state->selected_entity_id;

    if (selected_id < 0) return NULL;

    Entity *current = level->entities;
    while (current) {
        if (current->id == selected_id) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

void update_editor_camera(Editor_State *editor_state, Controller_State *controller_state,
                          bool32 has_focus, bool32 should_move,
                          real32 dt) {
    Camera *camera = &editor_state->camera;
    bool32 use_freecam = editor_state->use_freecam;
    Basis initial_basis = camera->initial_basis;

    if (use_freecam && has_focus) {
        real32 delta_x = controller_state->current_mouse.x - controller_state->last_mouse.x;
        real32 delta_y = controller_state->current_mouse.y - controller_state->last_mouse.y;

        real32 heading_delta = 0.2f * delta_x;
        real32 pitch_delta = 0.2f * delta_y;

        int32 heading_rotations = (int32) floorf((camera->heading + heading_delta) / 360.0f);
        int32 pitch_rotations = (int32) floorf((camera->pitch + pitch_delta) / 360.0f);
        camera->heading = (camera->heading + heading_delta) - heading_rotations*360.0f;
        camera->pitch = clamp(camera->pitch + pitch_delta, -90.0f, 90.0f);
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

        real32 camera_speed = Editor_Constants::camera_speed;
        movement_delta = normalize(movement_delta) * camera_speed;
        camera->position += movement_delta * dt;
    }
    
    Basis current_basis = { forward, right, up };
    camera->current_basis = current_basis;
}

Entity *pick_entity(Editor_State *editor_state, Ray cursor_ray) {
    Level *level = &game_state->level;

    Entity *picked_entity = NULL;
    real32 t_min = FLT_MAX;

    Basis camera_basis = editor_state->camera.current_basis;
    Vec3 plane_normal = -camera_basis.forward;

    Entity *current = level->entities;
    while (current) {
        // if we have an entity that is a light source and has a mesh, we don't pick against the mesh and instead
        // we pick against the light icon. we may want to do this, but we just do it like this for now since
        // we don't currently have entities that are lights and have a mesh.
        if (current->flags & ENTITY_LIGHT) {
            real32 plane_d = dot(current->transform.position, plane_normal);
            real32 t;
            if (ray_intersects_plane(cursor_ray, plane_normal, plane_d, &t)) {
                Vec3 hit_position = cursor_ray.origin + t*cursor_ray.direction;
                real32 plane_space_hit_x = dot(hit_position - current->transform.position, camera_basis.right);
                real32 plane_space_hit_y = dot(hit_position - current->transform.position, camera_basis.up);

                real32 icon_side_length = Editor_Constants::point_light_side_length;
                real32 offset = 0.5f * icon_side_length;
                if (plane_space_hit_x > -offset && plane_space_hit_x < offset &&
                    plane_space_hit_y > -offset && plane_space_hit_y < offset) {
                    if (t < t_min) {
                        t_min = t;
                        picked_entity = current;
                    }
                }
            }
        } else if (current->flags & ENTITY_MESH) {
            real32 aabb_t;
            Mesh *mesh = get_mesh(current->mesh_name);
            if (ray_intersects_aabb(cursor_ray, current->transformed_aabb, &aabb_t) && (aabb_t < t_min)) {
                Ray_Intersects_Mesh_Result result;
                if (ray_intersects_mesh(cursor_ray, mesh, current->transform, true, &result) &&
                    (result.t < t_min)) {
                    t_min = result.t;
                    picked_entity = current;
                }
            }
        }

        current = current->next;
    }

    return picked_entity;
}

void update_gizmo(Editor_State *editor_state) {
    if (editor_state->selected_entity_id < 0) return;

    Gizmo_State *gizmo_state = &editor_state->gizmo_state;
    Camera *camera = &editor_state->camera;

    real32 gizmo_scale_factor = distance(gizmo_state->transform.position - camera->position) / 5.0f;
    gizmo_state->transform.scale = make_vec3(gizmo_scale_factor, gizmo_scale_factor, gizmo_scale_factor);

    Entity *entity = get_selected_entity(editor_state);
    gizmo_state->transform.position = entity->transform.position;
    gizmo_state->transform.rotation = entity->transform.rotation;
}

Gizmo_Handle pick_gizmo(Editor_State *editor_state, Ray cursor_ray, real32 *t_result) {
    Gizmo_State *gizmo_state = &editor_state->gizmo_state;
    Transform_Mode transform_mode = gizmo_state->transform_mode;

    Entity *entity = get_selected_entity(editor_state);

    Transform x_transform, y_transform, z_transform;
    // TODO: maybe add some scale here? for a bit of extra space to click on the gizmo.
    if (transform_mode == TRANSFORM_GLOBAL) {
        x_transform = gizmo_state->transform;
        x_transform.rotation = make_quaternion();
        y_transform = gizmo_state->transform;
        y_transform.rotation = make_quaternion(90.0f, z_axis);
        z_transform = gizmo_state->transform;
        z_transform.rotation = make_quaternion(-90.0f, y_axis);
    } else {
        // local transform
        x_transform = gizmo_state->transform;
        y_transform = gizmo_state->transform;
        y_transform.rotation = gizmo_state->transform.rotation*make_quaternion(90.0f, z_axis);
        z_transform = gizmo_state->transform;
        z_transform.rotation = gizmo_state->transform.rotation*make_quaternion(-90.0f, y_axis);
    }

    Gizmo_Handle picked_handle = GIZMO_HANDLE_NONE;

    Transform gizmo_handle_transforms[3] = { x_transform, y_transform, z_transform };

    // check ray against translation arrows
    Gizmo_Handle gizmo_translation_handles[3] = { GIZMO_TRANSLATE_X, GIZMO_TRANSLATE_Y, GIZMO_TRANSLATE_Z };
    Mesh *arrow_mesh = get_mesh(gizmo_state->arrow_mesh_name);

    real32 t_min = FLT_MAX;
    for (int32 i = 0; i < 3; i++) {
        Transform gizmo_handle_transform = gizmo_handle_transforms[i];
        Ray_Intersects_Mesh_Result result;
        if (ray_intersects_mesh(cursor_ray, arrow_mesh, gizmo_handle_transform, true, &result) &&
            (result.t < t_min)) {
            t_min = result.t;
            picked_handle = gizmo_translation_handles[i];
        }
    }

    // check ray against scale cube handles
    Gizmo_Handle gizmo_scale_handles[3] = { GIZMO_SCALE_X, GIZMO_SCALE_Y, GIZMO_SCALE_Z };
    Mesh *gizmo_cube_mesh = get_mesh(gizmo_state->cube_mesh_name);

    for (int32 i = 0; i < 3; i++) {
        Transform gizmo_handle_transform = gizmo_handle_transforms[i];
        Ray_Intersects_Mesh_Result result;
        if (ray_intersects_mesh(cursor_ray, gizmo_cube_mesh, gizmo_handle_transform, true, &result) &&
            (result.t < t_min)) {
            t_min = result.t;
            picked_handle = gizmo_scale_handles[i];
        }
    }

    // check ray against rotation rings
    Gizmo_Handle gizmo_rotation_handles[3] = { GIZMO_ROTATE_X, GIZMO_ROTATE_Y, GIZMO_ROTATE_Z };
    Mesh *ring_mesh = get_mesh(gizmo_state->ring_mesh_name);

    for (int32 i = 0; i < 3; i++) {
        Transform gizmo_handle_transform = gizmo_handle_transforms[i];
        Ray_Intersects_Mesh_Result result;
        if (ray_intersects_mesh(cursor_ray, ring_mesh, gizmo_handle_transform, true, &result) &&
            (result.t < t_min)) {
            t_min = result.t;
            picked_handle = gizmo_rotation_handles[i];
        }
    }

    *t_result = t_min;

    return picked_handle;
}

Vec3 do_gizmo_translation(Editor_State *editor_state, Ray cursor_ray) {
    Gizmo_State *gizmo_state = &editor_state->gizmo_state;
    Camera *camera = &editor_state->camera;
    Vec3 camera_forward = camera->current_basis.forward;

    real32 t;
    
    Vec3 initial_hit;
    if (gizmo_state->transform_mode == TRANSFORM_GLOBAL) {
        initial_hit = gizmo_state->global_initial_gizmo_hit;
    } else {
        initial_hit = gizmo_state->local_initial_gizmo_hit;
    }

    Ray transform_ray = make_ray(initial_hit,
                                 gizmo_state->gizmo_transform_axis);
    Plane plane = get_plane_containing_ray(transform_ray, camera_forward);
    bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);

    Vec3 new_position = gizmo_state->original_entity_transform.position;

    // this will always intersect unless your FOV is >= 180 degrees
    if (intersects_plane) {
        Vec3 intersect_point = cursor_ray.origin + t*cursor_ray.direction;
        real32 delta_length = dot(intersect_point - initial_hit,
                                  gizmo_state->gizmo_transform_axis);
        Vec3 delta = gizmo_state->gizmo_transform_axis * delta_length;
        new_position += delta;
    }

    return new_position;
}

Vec3 do_gizmo_scale(Editor_State *editor_state, Ray cursor_ray) {
    Gizmo_State *gizmo_state = &editor_state->gizmo_state;
    Camera *camera = &editor_state->camera;
    Vec3 camera_forward = camera->current_basis.forward;

    real32 t;

    Vec3 initial_hit;
    if (gizmo_state->transform_mode == TRANSFORM_GLOBAL) {
        initial_hit = gizmo_state->global_initial_gizmo_hit;
    } else {
        initial_hit = gizmo_state->local_initial_gizmo_hit;
    }

    Gizmo_Handle gizmo_handle = gizmo_state->selected_gizmo_handle;
    Vec3 scale_transform_axis = {};
    if (gizmo_handle == GIZMO_SCALE_X) {
        scale_transform_axis = x_axis;
    } else if (gizmo_handle == GIZMO_SCALE_Y) {
        scale_transform_axis = y_axis;
    } else {
        scale_transform_axis = z_axis;
    }

    // we still use the transform ray here since the selected gizmo axis is lined up with the transform ray.
    Ray transform_ray = make_ray(initial_hit,
                                 gizmo_state->gizmo_transform_axis);
    Plane plane = get_plane_containing_ray(transform_ray, camera_forward);
    bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);

    Transform original_transform = gizmo_state->original_entity_transform;
    Vec3 new_scale = original_transform.scale;

    // this will always intersect unless your FOV is >= 180 degrees
    if (intersects_plane) {
        Vec3 intersect_point = cursor_ray.origin + t*cursor_ray.direction;
        real32 delta_length = dot(intersect_point - initial_hit,
                                  gizmo_state->gizmo_transform_axis);
        Vec3 delta = scale_transform_axis * delta_length;
        new_scale += delta;
        if (gizmo_state->transform_mode == TRANSFORM_LOCAL) {
            return new_scale;
        } else {
            Mat4 model = get_model_matrix(original_transform);
            delta = make_vec3(1.0f, 1.0f, 1.0f) + delta;

            // NOTE: when delta goes below the zero vector, the entity grows instead of shrinks. i'm pretty sure
            //       it's because when we call distance() on the columns of the scaled model matrix, it does not
            //       give signed values. i think this is fine.
            Mat4 scale_matrix = make_scale_matrix(delta);
            Mat4 scaled_model = scale_matrix * model;
            new_scale = make_vec3(distance(truncate_v4_to_v3(scaled_model.col1)),
                                  distance(truncate_v4_to_v3(scaled_model.col2)),
                                  distance(truncate_v4_to_v3(scaled_model.col3)));
        }
    }

    return new_scale;
}

Quaternion do_gizmo_rotation(Editor_State *editor_state, Ray cursor_ray) {
    Gizmo_State *gizmo_state = &editor_state->gizmo_state;
    Camera *camera = &editor_state->camera;

    real32 t;
    Vec3 camera_forward = camera->current_basis.forward;
    Vec3 center = gizmo_state->transform.position;
    Plane plane = { dot(center, gizmo_state->gizmo_transform_axis),
                    gizmo_state->gizmo_transform_axis };
    bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);

    Vec3 initial_hit;
    if (gizmo_state->transform_mode == TRANSFORM_GLOBAL) {
        initial_hit = gizmo_state->global_initial_gizmo_hit;
    } else {
        initial_hit = gizmo_state->local_initial_gizmo_hit;
    }

    Quaternion new_rotation = gizmo_state->original_entity_transform.rotation;

    // this will always intersect unless your FOV is >= 180 degrees
    if (intersects_plane) {
        Vec3 intersect_point = cursor_ray.origin + t*cursor_ray.direction;

        Vec3 center_to_intersect_point = intersect_point - center;
        Vec3 center_to_last_intersect_point = initial_hit - center;

        if (are_collinear(normalize(center_to_last_intersect_point), normalize(center_to_intersect_point))) {
            return new_rotation;
        }

        Vec3 out_vector = cross(gizmo_state->gizmo_transform_axis, center_to_last_intersect_point);
        
        real32 sign = 1.0f;
        if (dot(center_to_intersect_point, out_vector) < 0.0f) sign = -1.0f;

        real32 a = distance(center_to_intersect_point);
        real32 b = distance(center_to_last_intersect_point);
        real32 c = distance(intersect_point - initial_hit);

        real32 angle_delta_degrees = sign * cosine_law_degrees(a, b, c);

        Quaternion delta_from_start = make_quaternion(angle_delta_degrees, gizmo_state->gizmo_transform_axis);
        new_rotation = delta_from_start * new_rotation;
    }

    return new_rotation;
}

void reset_editor(Editor_State *editor_state) {
    editor_state->selected_entity_id = -1;
    editor_state->last_selected_entity_id = -1;

    reset_asset_library_state();
}

#if 0
void debug_check_collisions(Level *level, Normal_Entity *entity) {
    FOR_LIST_NODES(Entity *, level->entities) {
#if 0
        if (current_node->value->type != ENTITY_NORMAL || current_node->value->id == entity->id) continue;

        Normal_Entity *entity_to_check = (Normal_Entity *) current_node->value;
        Mesh mesh = get_mesh(asset_manager, entity_to_check->mesh_id);
        Vec3 penetration_normal;
        real32 penetration_depth;
        Vec3 penetration_point;
        int32 triangle_index;
        Vec3 triangle_normal;
        bool32 intersected = capsule_intersects_mesh(entity->collider.capsule.capsule, mesh,
                                                     entity_to_check->transform,
                                                     &penetration_normal, &penetration_depth, &penetration_point,
                                                     &triangle_index, &triangle_normal);
#endif

        Entity *uncast_entity = current_node->value;
        if (uncast_entity->type != ENTITY_NORMAL) continue;

        Normal_Entity *entity_to_check = (Normal_Entity *) current_node->value;
        Mat4 object_to_world = get_model_matrix(entity_to_check->transform);
        Mesh *mesh = get_mesh_pointer(asset_manager, entity_to_check->mesh_id);

        Capsule player_capsule = entity->collider.capsule.capsule;
        for (int32 i = 0; i < (int32) mesh->num_triangles; i++) {
            Vec3 triangle[3];
            get_triangle(mesh, i, triangle);
            transform_triangle(triangle, &object_to_world);
            Vec3 triangle_normal = get_triangle_normal(triangle);

            Vec3 penetration_normal, penetration_point;
            real32 penetration_depth;

            bool32 intersected = capsule_intersects_triangle(player_capsule, triangle,
                                                             &penetration_normal,
                                                             &penetration_depth,
                                                             &penetration_point);
            real32 penetration_height = penetration_point.y - entity->transform.position.y;
            if (intersected) {
                //player->position += (penetration_normal * (player_capsule.radius - penetration_depth));

#if DEBUG_SHOW_COLLISION_LINES
                Vec4 triangle_color = make_vec4(1.0f, 1.0f, 1.0f, 1.0f);
                add_debug_line(&Context::game_state->debug_state,
                               triangle[0], triangle[1], triangle_color);
                add_debug_line(&Context::game_state->debug_state,
                               triangle[1], triangle[2], triangle_color);
                add_debug_line(&Context::game_state->debug_state,
                               triangle[2], triangle[0], triangle_color);

                add_debug_line(&Context::game_state->debug_state,
                               entity->transform.position, penetration_point,
                               make_vec4(1.0f, 0.0f, 0.0f, 1.0f));
#endif
            }
        }

        //if (intersected) return true;
    }

#if 0
    Marker m = begin_region();
    Allocator *frame_allocator = (Allocator *) &memory.frame_arena;
    String_Buffer buffer = make_string_buffer(frame_allocator, 1024);
    FOR_LIST_NODES(Entity *, level->entities) {
        if (current_node->value->type != ENTITY_NORMAL || current_node->value->id == entity->id) continue;
        Normal_Entity *entity_to_check = (Normal_Entity *) current_node->value;

        Mat4 object_to_world = get_model_matrix(entity_to_check->transform);

        Mesh *mesh = get_mesh_pointer(asset_manager, entity_to_check->mesh_id);

        for (int32 i = 0; i < (int32) mesh->num_triangles; i++) {
            Vec3 triangle[3];
            get_triangle(mesh, i, triangle);
            transform_triangle(triangle, &object_to_world);
            Vec3 triangle_normal = get_triangle_normal(triangle);

            if (triangle_normal.y < 0.2f) continue;
            Vec3 point_on_triangle_plane = get_point_on_plane_from_xz(entity->transform.position.x,
                                                                      entity->transform.position.z,
                                                                      triangle_normal, triangle[0]);
            char *str = string_format(temp_region, 128,
                                      "entity %d; triangle %d; point on plane: %f\n", entity_to_check->id, i, point_on_triangle_plane.y);
            append_string(&buffer, str);
        }
    }

    int32 font_id;
    Font font = get_font(asset_manager, "calibri14", &font_id);
    do_text(Context::ui_manager, 900.0f, 350.0f, to_char_array(frame_allocator, buffer), font_id,
            "triangle plane points");
    end_region(m);
#endif

    //return false;
}
#endif

void update_editor(Controller_State *controller_state, real32 dt) {
    //UI_Manager *ui_manager = &game_state->ui_manager;
    Editor_State *editor_state = &game_state->editor_state;
    Gizmo_State *gizmo_state = &editor_state->gizmo_state;
    Display_Output *display_output = &render_state->display_output;

    if (just_pressed(controller_state->key_tab) && !ui_has_focus()) {
        editor_state->use_freecam = !editor_state->use_freecam;
        platform_set_cursor_visible(!editor_state->use_freecam);
    }
    
    bool32 camera_should_move = editor_state->use_freecam && !ui_has_focus();
    update_editor_camera(editor_state, controller_state,
                         platform_window_has_focus(), camera_should_move, dt);
    update_render_state(editor_state->camera);

    if (editor_state->use_freecam && platform_window_has_focus()) {
        Vec2 center = make_vec2(display_output->width / 2.0f, display_output->height / 2.0f);
        platform_set_cursor_pos(center);
        controller_state->current_mouse = center;
    }

    if (editor_state->use_freecam) {
        ui_disable_input();
    } else {
        ui_enable_input();
    }

    if (just_pressed(controller_state->key_z)) {
        editor_state->show_wireframe = !editor_state->show_wireframe;
    }

    if (just_pressed(controller_state->key_x)) {
        if (gizmo_state->transform_mode == TRANSFORM_GLOBAL) {
            gizmo_state->transform_mode = TRANSFORM_LOCAL;
        } else {
            gizmo_state->transform_mode = TRANSFORM_GLOBAL;
        }

        if (gizmo_state->selected_gizmo_handle != GIZMO_HANDLE_NONE) {
            Entity *selected_entity = get_selected_entity(editor_state);
            assert(selected_entity);
            set_entity_transform(selected_entity, gizmo_state->original_entity_transform);
        }
    }

    // mesh picking
    Vec3 cursor_world_space = cursor_pos_to_world_space(controller_state->current_mouse);
    Ray cursor_ray = { cursor_world_space,
                       normalize(cursor_world_space - render_state->camera.position) };
    
    if (!ui_has_hot() &&
        !ui_has_active() &&
        !editor_state->use_freecam && was_clicked(controller_state->left_mouse)) {
        if (!gizmo_state->selected_gizmo_handle) {
            Entity *entity = pick_entity(editor_state, cursor_ray);
            //editor_state->selected_entity_id = entity->id;
            
            if (entity) {
                if (entity->id != editor_state->selected_entity_id) {
                    editor_state->last_selected_entity_id = editor_state->selected_entity_id;
                    editor_state->selected_entity_id = entity->id;
                    editor_state->selected_entity_modified = true;

                    gizmo_state->transform = entity->transform;
                }
            } else {
                editor_state->selected_entity_id = -1;
            }
        }
    }

    update_gizmo(editor_state);

    // gizmo picking
    if (editor_state->selected_entity_id >= 0 &&
        !ui_has_hot() &&
        !gizmo_state->selected_gizmo_handle) {
        real32 pick_gizmo_t;
        Gizmo_Handle picked_handle = pick_gizmo(editor_state, cursor_ray, &pick_gizmo_t);
        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
            if (picked_handle) {
                Entity *selected_entity = get_selected_entity(editor_state);
#if 0
                editor_state->old_entity = allocate_and_copy_entity(editor_state->history.allocator_pointer,
                                                                    selected_entity);
#endif
                Vec3 global_transform_axis = get_gizmo_transform_axis(TRANSFORM_GLOBAL,
                                                                      picked_handle,
                                                                      selected_entity->transform);
                Vec3 local_transform_axis = get_gizmo_transform_axis(TRANSFORM_LOCAL,
                                                                     picked_handle,
                                                                     selected_entity->transform);
                gizmo_state->local_initial_gizmo_hit = get_gizmo_hit(gizmo_state,
                                                                     picked_handle, pick_gizmo_t,
                                                                     cursor_ray, local_transform_axis);
                gizmo_state->global_initial_gizmo_hit = get_gizmo_hit(gizmo_state,
                                                                      picked_handle, pick_gizmo_t,
                                                                      cursor_ray, global_transform_axis);
                gizmo_state->original_entity_transform = selected_entity->transform;

                //start_entity_change(editor_state, selected_entity);
            }

            gizmo_state->selected_gizmo_handle = picked_handle;
        } else {
            gizmo_state->hovered_gizmo_handle = picked_handle;
        }
    }

    if (editor_state->use_freecam ||
        (ui_has_hot() && !controller_state->left_mouse.is_down)) {
        gizmo_state->hovered_gizmo_handle = GIZMO_HANDLE_NONE;
        gizmo_state->selected_gizmo_handle = GIZMO_HANDLE_NONE;
    }

    if (gizmo_state->selected_gizmo_handle) {
        ui_disable_input();
        Entity *entity = get_selected_entity(editor_state);
        if (controller_state->left_mouse.is_down) {
            gizmo_state->gizmo_transform_axis = get_gizmo_transform_axis(gizmo_state->transform_mode,
                                                                          gizmo_state->selected_gizmo_handle,
                                                                          gizmo_state->original_entity_transform);

            if (is_translation(gizmo_state->selected_gizmo_handle)) {
                Vec3 new_position = do_gizmo_translation(editor_state, cursor_ray);
                update_entity_position(entity, new_position);
            } else if (is_rotation(gizmo_state->selected_gizmo_handle)) {
                Quaternion new_rotation = do_gizmo_rotation(editor_state, cursor_ray);
                update_entity_rotation(entity, new_rotation);
            } else if (is_scale(gizmo_state->selected_gizmo_handle)) {
                Vec3 new_scale = do_gizmo_scale(editor_state, cursor_ray);
                update_entity_scale(entity, new_scale);
            } else {
                assert(!"Should be unreachable.");
            }

            gizmo_state->transform.position = entity->transform.position;
            gizmo_state->transform.rotation = entity->transform.rotation;

            editor_state->selected_entity_modified = true;
        } else {
            gizmo_state->selected_gizmo_handle = GIZMO_HANDLE_NONE;
            //end_entity_change(editor_state, entity);
        }
    }

    update_gizmo(editor_state);

    Level *level = &game_state->level;
    // debug
    #if 0
    FOR_LIST_NODES(Entity *, level->entities) {
        Entity *uncast_entity = current_node->value;
        if (uncast_entity->type != ENTITY_NORMAL) continue;
        Normal_Entity *entity = (Normal_Entity *) current_node->value;
        if (entity->collider.type == Collider_Type::CAPSULE) {
            debug_check_collisions(asset_manager, level, entity);
            // we only have one entity that has a capsule collider, so break
            break;
        }
    }
    #endif
}

void draw_entity_box_2(bool32 force_reset) {
    UI_Window_Theme window_theme = DEFAULT_WINDOW_THEME;
    window_theme.size_type = { UI_SIZE_ABSOLUTE, UI_SIZE_FIT_CHILDREN };
    window_theme.semantic_size = { 200.0f, 0.0f };
    
    push_window("Entity Properties", window_theme,
                "entity-properties-window", "entity-properties-window-title-bar");

    Editor_State *editor_state = &game_state->editor_state;
    assert(editor_state->selected_entity_id > -1);
    Entity *entity = get_entity(&game_state->level, editor_state->selected_entity_id);

    UI_Container_Theme content_theme = {
        { 5.0f, 5.0f, 5.0f, 5.0f },
        DEFAULT_BOX_BACKGROUND,
        UI_POSITION_NONE,
        {},
        { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN },
        { 0.0f, 0.0f },
        UI_LAYOUT_VERTICAL
    };

    UI_Text_Field_Theme transform_field_theme = {
        DEFAULT_BUTTON_BACKGROUND, DEFAULT_BUTTON_HOT_BACKGROUND, DEFAULT_BUTTON_ACTIVE_BACKGROUND,
        make_vec4(0.0f, 1.0f, 0.0f, 1.0f),
        0.0f, 0, 0, {}, 0.0f,
        default_font,
        { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN },
        { 1.0f, 20.0f }
    };

    UI_Text_Field_Slider_Theme slider_theme = editor_slider_theme;
    
    ui_push_container(content_theme, "entity-properties-content");
    {
        UI_Theme row_theme = {};
        row_theme.size_type      = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
        row_theme.semantic_size  = { 1.0f, 0.0f };
        row_theme.layout_type    = UI_LAYOUT_HORIZONTAL;
        //row_theme.background_color = rgb_to_vec4(0, 255, 0); // debugging

        UI_Theme label_theme = {};
        label_theme.size_type = { UI_SIZE_ABSOLUTE, UI_SIZE_PERCENTAGE };
        label_theme.layout_type = UI_LAYOUT_CENTER;
        label_theme.semantic_size = { 20.0f, 1.0f };
        label_theme.background_color = rgb_to_vec4(50, 50, 60);
        //label_theme.border_flags = BORDER_BOTTOM;
        //label_theme.border_color = rgb_to_vec4(24, 24, 28);
        //label_theme.border_width = 1.0f;

        Transform *transform = &entity->transform;
        Vec3 new_position = transform->position;

        do_text("Position");
        ui_y_pad(1.0f);

        ui_add_and_push_widget("", row_theme);
        {
            ui_push_widget("", label_theme, UI_WIDGET_DRAW_BACKGROUND);
            { do_text("X"); }
            ui_pop_widget();
            
            ui_x_pad(1.0f);
            new_position.x = do_text_field_slider(transform->position.x, slider_theme,
                                                  "position-x-slider", "position-x-slider-text",
                                                  force_reset);
        } ui_pop_widget();

        ui_y_pad(1.0f);
        ui_add_and_push_widget("", row_theme);
        {
            ui_push_widget("", label_theme, UI_WIDGET_DRAW_BACKGROUND);
            { do_text("Y"); }
            ui_pop_widget();

            ui_x_pad(1.0f);
            new_position.y = do_text_field_slider(transform->position.y, slider_theme,
                                                  "position-y-slider", "position-y-slider-text",
                                                  force_reset);
        } ui_pop_widget();

        ui_y_pad(1.0f);
        ui_add_and_push_widget("", row_theme);
        {
            ui_push_widget("", label_theme, UI_WIDGET_DRAW_BACKGROUND);
            { do_text("Z"); }
            ui_pop_widget();

            ui_x_pad(1.0f);
            new_position.z = do_text_field_slider(transform->position.z, slider_theme,
                                                  "position-z-slider", "position-z-slider-text",
                                                  force_reset);
        } ui_pop_widget();

        update_entity_position(entity, new_position);
        ui_y_pad(5.0f);

        // these controls are kind of weird, but i don't really think it's a problem.
        // i think usually if you're doing rotations, you'll just use the gizmos.
        // they're weird because you have to be aware of the euler rotation order (roll, pitch, heading
        // or z, x, y) to know what the result of your inputs will be.
        Vec3 euler_rotation;
        get_euler_angles_from_quaternion(transform->rotation, &euler_rotation);
        real32 roll_degs, pitch_degs, heading_degs;
        
        do_text("Rotation");
        ui_y_pad(1.0f);

        ui_add_and_push_widget("", row_theme);
        {
            ui_push_widget("", label_theme, UI_WIDGET_DRAW_BACKGROUND);
            { do_text("X"); }
            ui_pop_widget();
            
            ui_x_pad(1.0f);
            pitch_degs = do_text_field_slider(rads_to_degs(euler_rotation.x), slider_theme,
                                              "rotation-x-slider", "rotation-x-slider-text",
                                              force_reset);
        } ui_pop_widget();

        ui_y_pad(1.0f);
        ui_add_and_push_widget("", row_theme);
        {
            ui_push_widget("", label_theme, UI_WIDGET_DRAW_BACKGROUND);
            { do_text("Y"); }
            ui_pop_widget();

            ui_x_pad(1.0f);
            heading_degs = do_text_field_slider(rads_to_degs(euler_rotation.y), slider_theme,
                                                "rotation-y-slider", "rotation-y-slider-text",
                                                force_reset);
        } ui_pop_widget();

        ui_y_pad(1.0f);
        ui_add_and_push_widget("", row_theme);
        {
            ui_push_widget("", label_theme, UI_WIDGET_DRAW_BACKGROUND);
            { do_text("Z"); }
            ui_pop_widget();

            ui_x_pad(1.0f);
            roll_degs = do_text_field_slider(rads_to_degs(euler_rotation.z), slider_theme,
                                             "rotation-z-slider", "rotation-z-slider-text",
                                             force_reset);
        } ui_pop_widget();

        Quaternion new_rotation = make_quaternion(roll_degs, pitch_degs, heading_degs);
        update_entity_rotation(entity, new_rotation);
        ui_y_pad(5.0f);

        Vec3 new_scale;
            
        do_text("Scale");
        ui_y_pad(1.0f);

        ui_add_and_push_widget("", row_theme);
        {
            ui_push_widget("", label_theme, UI_WIDGET_DRAW_BACKGROUND);
            { do_text("X"); }
            ui_pop_widget();
            
            ui_x_pad(1.0f);
            new_scale.x = do_text_field_slider(transform->scale.x, slider_theme,
                                               "scale-x-slider", "scale-x-slider-text",
                                               force_reset);
        } ui_pop_widget();

        ui_y_pad(1.0f);
        ui_add_and_push_widget("", row_theme);
        {
            ui_push_widget("", label_theme, UI_WIDGET_DRAW_BACKGROUND);
            { do_text("Y"); }
            ui_pop_widget();

            ui_x_pad(1.0f);
            new_scale.y = do_text_field_slider(transform->scale.y, slider_theme,
                                               "scale-y-slider", "scale-y-slider-text",
                                               force_reset);
        } ui_pop_widget();

        ui_y_pad(1.0f);
        ui_add_and_push_widget("", row_theme);
        {
            ui_push_widget("", label_theme, UI_WIDGET_DRAW_BACKGROUND);
            { do_text("Z"); }
            ui_pop_widget();

            ui_x_pad(1.0f);
            new_scale.z = do_text_field_slider(transform->scale.z, slider_theme,
                                               "scale-z-slider", "scale-z-slider-text",
                                               force_reset);
        } ui_pop_widget();

        update_entity_scale(entity, new_scale);

        if (entity->flags & ENTITY_MATERIAL) {
            ui_y_pad(10.0f);
            do_text("Material");
            ui_y_pad(1.0f);
        
            Material *material = get_material(entity->material_id);

            const int32 MAX_MATERIAL_NAMES = 256;
            char *material_names[MAX_MATERIAL_NAMES];
            int32 num_material_names;
            get_material_names((Allocator *) &ui_manager->frame_arena, material_names, MAX_MATERIAL_NAMES,
                               &num_material_names);

            int32 selected_index = -1;
            for (int32 i = 0; i < num_material_names; i++) {
                if (string_equals(material->name, material_names[i])) {
                    selected_index = i;
                }
            }

            UI_Dropdown_Theme dropdown_theme = {};
            dropdown_theme.button_theme = editor_button_theme;
            dropdown_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN };
            dropdown_theme.size = { 1.0f, 0.0f };
            dropdown_theme.item_theme = editor_dropdown_item_theme;
            dropdown_theme.selected_item_theme = editor_selected_dropdown_item_theme;
        
            int32 dropdown_selected_index = do_dropdown(dropdown_theme,
                                                        material_names, num_material_names,
                                                        selected_index,
                                                        "material_dropdown_button", "material_dropdown",
                                                        "material_dropdown_inner", "material_dropdown_item",
                                                        force_reset);
            if (dropdown_selected_index != selected_index) {
                selected_index = dropdown_selected_index;
                set_material(entity, material_names[dropdown_selected_index]);
                editor_state->selected_entity_modified = true;
            }
        }
    }
    ui_pop_widget();
    
    ui_pop_widget();
}

int32 get_selected_name_index(String name, char **names, int32 num_names) {
    for (int32 i = 0; i < num_names; i++) {
        if (string_equals(name, names[i])) {
            return i;
        }
    }

    return -1;
}

void do_y_centered_text(char *text) {
    UI_Theme y_center_text_theme = {};
    y_center_text_theme.size_type = { UI_SIZE_FIT_CHILDREN, UI_SIZE_FILL_REMAINING };
    y_center_text_theme.semantic_size = { 0.0f, 1.0f };
    y_center_text_theme.layout_type = UI_LAYOUT_CENTER;
    
    ui_add_and_push_widget("", y_center_text_theme);
    {
        do_text(text);
    }
    ui_pop_widget();
}

void draw_asset_library() {
    UI_Window_Theme window_theme = DEFAULT_WINDOW_THEME;

    // since DEFAULT_WINDOW_THEME is FIT_CHILDREN, this is a minimum size
    window_theme.size_type = { UI_SIZE_ABSOLUTE, UI_SIZE_ABSOLUTE };
    window_theme.semantic_size = { 500.0f, 500.0f };
    
    push_window("Asset Library", window_theme,
                "asset-library-window", "asset-library-window-title-bar");

    Editor_State *editor_state = &game_state->editor_state;
    Asset_Library_State *asset_library_state = &editor_state->asset_library_state;
    
    UI_Container_Theme content_theme = {
        { 5.0f, 5.0f, 5.0f, 5.0f },
        DEFAULT_BOX_BACKGROUND,
        UI_POSITION_NONE,
        {},
        { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING },
        {},
        UI_LAYOUT_HORIZONTAL
    };

    UI_Theme list_container_theme = NULL_THEME;
    list_container_theme.layout_type = UI_LAYOUT_VERTICAL;
    list_container_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_PERCENTAGE };
    list_container_theme.semantic_size = { 0.4f, 1.0f };

    UI_Theme properties_container_theme = list_container_theme;
    properties_container_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_PERCENTAGE };
    properties_container_theme.semantic_size = { 0.0f, 1.0f };
    properties_container_theme.background_color = rgb_to_vec4(255, 0, 0);
    //properties_container_theme.layout_type = UI_LAYOUT_VERTICAL_SPACE_BETWEEN;
    
    UI_Theme list_theme = NULL_THEME;
    list_theme.layout_type = UI_LAYOUT_VERTICAL;
    list_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
    list_theme.background_color = { 0.0f, 0.0f, 0.0f, 1.0f };

    UI_Dropdown_Theme dropdown_theme = {};
    dropdown_theme.button_theme = editor_button_theme;
    dropdown_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN };
    dropdown_theme.size = { 1.0f, 0.0f };
    dropdown_theme.item_theme = editor_dropdown_item_theme;
    dropdown_theme.selected_item_theme = editor_selected_dropdown_item_theme;

    UI_Theme row_theme = {};
    row_theme.size_type = { UI_SIZE_FIT_CHILDREN, UI_SIZE_FIT_CHILDREN };
    row_theme.semantic_size = { 0.0f, 0.0f };
    row_theme.layout_type = UI_LAYOUT_HORIZONTAL;

    UI_Theme full_row_theme = row_theme;
    full_row_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
    full_row_theme.layout_type = UI_LAYOUT_HORIZONTAL_SPACE_BETWEEN;

    const int32 MAX_MATERIALS = 256;
    int32 material_ids[MAX_MATERIALS];
    int32 num_materials_listed = 0;
    
    ui_push_container(content_theme, "asset-library-content");
    {
        Material *selected_material = NULL;
        
        ui_add_and_push_widget("asset-library-material-list-container", list_container_theme);
        {
            ui_add_and_push_widget("", full_row_theme);
            {
                do_y_centered_text("Materials");
                UI_Button_Theme add_button_theme = editor_button_theme;
                add_button_theme.size_type.x = UI_SIZE_ABSOLUTE;
                add_button_theme.size.x = 20.0f;

                bool32 add_material_clicked = do_text_button("+", add_button_theme, "add_material_button");
                if (add_material_clicked) {
                    Material_Info material_info = default_material_info;
                    Marker m = begin_region();
                    bool32 gen_result = generate_asset_name(temp_region, "Material", 256, &material_info.name);
                    assert(gen_result);
                    
                    Material *new_material = add_material(&material_info, Material_Type::LEVEL);
                    asset_library_state->selected_material_id = new_material->id;
                    asset_library_state->material_modified = true;
                    
                    end_region(m);
                }
            } ui_pop_widget();

            ui_y_pad(5.0f);

            UI_Scrollable_Region_Theme list_scroll_region_theme = {};
            list_scroll_region_theme.background_color = rgb_to_vec4(0, 0, 0);
            list_scroll_region_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };

            push_scrollable_region(list_scroll_region_theme, 0,
                                   "material-list-scroll-region", "material-list-scroll-region-scrollbar",
                                   "material-list-scroll-region-handle");
            ui_add_and_push_widget("asset-library-material-list-container", list_theme, UI_WIDGET_DRAW_BACKGROUND);
            {
                UI_Button_Theme item_theme = editor_button_theme;

                UI_Button_Theme selected_item_theme = item_theme;
                selected_item_theme.background_color = rgb_to_vec4(61, 96, 252);
                selected_item_theme.hot_background_color = selected_item_theme.background_color;
                selected_item_theme.active_background_color = selected_item_theme.background_color;
                
                // don't use i for the button indices because that's for buckets and
                // not the actual materials we've visited
                for (int32 i = 0; i < NUM_MATERIAL_BUCKETS; i++) {
                    Material *current = asset_manager->material_table[i];
                    while (current) {
                        if (current->type == Material_Type::LEVEL) {
                            assert(num_materials_listed < MAX_MATERIALS);
                            material_ids[num_materials_listed] = current->id;
                            
                            if (num_materials_listed == 0 && asset_library_state->selected_material_id < 0) {
                                // select the first one by default
                                asset_library_state->selected_material_id = current->id;
                            }
                            
                            char *material_name = to_char_array((Allocator *) &ui_manager->frame_arena,
                                                                current->name);

                            bool32 is_selected = current->id == asset_library_state->selected_material_id;
                            bool32 pressed = do_text_button(material_name,
                                                            is_selected ? selected_item_theme : item_theme,
                                                            "asset-library-material-list-item",
                                                            num_materials_listed);

                            if (pressed || is_selected) {
                                if (asset_library_state->selected_material_id != current->id) {
                                    asset_library_state->selected_material_id = current->id;
                                    asset_library_state->material_modified = true;
                                }

                                selected_material = current;
                            }

                            num_materials_listed++;
                        }

                        current = current->table_next;
                    }
                }
            }
            ui_pop_widget(); // asset-library-material-list-container
            ui_pop_widget(); // scrollable region
        }
        ui_pop_widget();

        ui_x_pad(10.0f);

        ui_add_and_push_widget("asset-library-material-info-container", properties_container_theme);
        {

            UI_Theme section_theme = {};
            section_theme.layout_type = UI_LAYOUT_VERTICAL;
            section_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING };

            ui_add_and_push_widget("", section_theme);
            {
                do_text("Properties");

                if (selected_material) {
                    ui_y_pad(10.0f);
                    {
                        do_text("Material Name");
                        UI_Text_Field_Theme field_theme = editor_text_field_theme;
                        field_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
                        field_theme.size.x = 0.0f;
                        UI_Text_Field_Result name_result = do_text_field(field_theme, selected_material->name,
                                                                         asset_library_state->material_modified,
                                                                         "material_name_text_field",
                                                                         "material_name_text_field_text");

                        if (name_result.committed) {
                            if (!string_equals(name_result.text, selected_material->name) && name_result.text.length > 0) {
                                if (!material_exists(name_result.text)) {
                                    replace_contents(&selected_material->name, name_result.text);
                                } else {
                                    add_message(Context::message_manager, make_string("Material name already exists!"));
                                }
                            }

                            asset_library_state->material_modified = true;
                        }
                    }

                    const int32 MAX_TEXTURE_NAMES = 256;
                    char *texture_names[MAX_TEXTURE_NAMES];
                    int32 num_texture_names;
                    get_texture_names((Allocator *) &ui_manager->frame_arena, texture_names, MAX_TEXTURE_NAMES,
                                      &num_texture_names);
            
                    ui_y_pad(10.0f);
                    {
                        ui_add_and_push_widget("", row_theme);
                        {
                            do_y_centered_text("Use Albedo Texture");
                            ui_x_pad(5.0f);
                            bool32 checked = do_checkbox(selected_material->flags & MATERIAL_USE_ALBEDO_TEXTURE,
                                                         editor_checkbox_theme, "material-albedo-texture-checkbox");
                            selected_material->flags = set_bits(selected_material->flags, MATERIAL_USE_ALBEDO_TEXTURE,
                                                                checked);
                        }
                        ui_pop_widget();
                        ui_y_pad(5.0f);

                        if (selected_material->flags & MATERIAL_USE_ALBEDO_TEXTURE) {
                            do_text("Albedo Texture");
                            int32 selected_index = get_selected_name_index(selected_material->albedo_texture_name,
                                                                           texture_names, num_texture_names);
                            assert(selected_index >= 0);
                            int32 dropdown_selected_index = do_dropdown(dropdown_theme,
                                                                        texture_names, num_texture_names,
                                                                        selected_index,
                                                                        "albedo_texture_dropdown_button",
                                                                        "albedo_texture_dropdown",
                                                                        "albedo_texture_dropdown_inner",
                                                                        "albedo_texture_dropdown_item",
                                                                        asset_library_state->material_modified);
                            if (dropdown_selected_index != selected_index) {
                                selected_index = dropdown_selected_index;
                                replace_contents(&selected_material->albedo_texture_name, texture_names[selected_index]);
                            }
                        } else {
                            UI_Interact_Result interact_result;
                            bool32 open_color_picker_pressed = do_text_button("Open Color Picker",
                                                                              editor_button_theme,
                                                                              "open-color-picker", 0,
                                                                              &interact_result);
                            if (open_color_picker_pressed) {
                                asset_library_state->material_albedo_color_picker_open = !asset_library_state->material_albedo_color_picker_open;
                            }

                            if (asset_library_state->material_albedo_color_picker_open) {
                                UI_Color_Picker_Result result = do_color_picker(selected_material->albedo_color,
                                                                                "albedo-color-picker",
                                                                                "albedo-color-picker-panel",
                                                                                "albedo-color-picker-slider",
                                                                                false);

                                if (result.should_hide && !interact_result.just_pressed) {
                                    asset_library_state->material_albedo_color_picker_open = false;
                                } else {
                                    selected_material->albedo_color = result.color;
                                }
                            }
                        }
                    }

                    ui_y_pad(10.0f);
                    {
                        ui_add_and_push_widget("", row_theme);
                        {
                            do_y_centered_text("Use Metalness Texture");
                            ui_x_pad(5.0f);
                            bool32 checked = do_checkbox(selected_material->flags & MATERIAL_USE_METALNESS_TEXTURE,
                                                         editor_checkbox_theme, "material-metalness-texture-checkbox");
                            selected_material->flags = set_bits(selected_material->flags,
                                                                MATERIAL_USE_METALNESS_TEXTURE,
                                                                checked);
                        }
                        ui_pop_widget();
                        ui_y_pad(5.0f);

                        if (selected_material->flags & MATERIAL_USE_METALNESS_TEXTURE) {
                            do_text("Metalness Texture");
                            int32 selected_index = get_selected_name_index(selected_material->metalness_texture_name,
                                                                           texture_names, num_texture_names);
                            assert(selected_index >= 0);
                            int32 dropdown_selected_index = do_dropdown(dropdown_theme,
                                                                        texture_names, num_texture_names,
                                                                        selected_index,
                                                                        "metalness_texture_dropdown_button",
                                                                        "metalness_texture_dropdown",
                                                                        "metalness_texture_dropdown_inner",
                                                                        "metalness_texture_dropdown_item",
                                                                        asset_library_state->material_modified);
                            if (dropdown_selected_index != selected_index) {
                                selected_index = dropdown_selected_index;
                                replace_contents(&selected_material->metalness_texture_name, texture_names[selected_index]);
                            }
                        } else {
                            do_text("Metalness");
                            selected_material->metalness = do_text_field_slider(selected_material->metalness,
                                                                                0.0f, 1.0f,
                                                                                editor_slider_theme,
                                                                                "material-metalness-slider",
                                                                                "material-metalness-slider",
                                                                                asset_library_state->material_modified);
                        }
                        ui_y_pad(5.0f);
                    }

                    ui_y_pad(10.0f);
                    {
                        ui_add_and_push_widget("", row_theme);
                        {
                            do_y_centered_text("Use Roughness Texture");
                            ui_x_pad(5.0f);
                            bool32 checked = do_checkbox(selected_material->flags & MATERIAL_USE_ROUGHNESS_TEXTURE,
                                                         editor_checkbox_theme, "material-roughness-texture-checkbox");
                            selected_material->flags = set_bits(selected_material->flags,
                                                                MATERIAL_USE_ROUGHNESS_TEXTURE,
                                                                checked);
                        }
                        ui_pop_widget();
                        ui_y_pad(5.0f);
                    
                        if (selected_material->flags & MATERIAL_USE_ROUGHNESS_TEXTURE) {
                            do_text("Roughness Texture");
                            int32 selected_index = get_selected_name_index(selected_material->roughness_texture_name,
                                                                           texture_names, num_texture_names);
                            assert(selected_index >= 0);
                            int32 dropdown_selected_index = do_dropdown(dropdown_theme,
                                                                        texture_names, num_texture_names,
                                                                        selected_index,
                                                                        "roughness_texture_dropdown_button",
                                                                        "roughness_texture_dropdown",
                                                                        "roughness_texture_dropdown_inner",
                                                                        "roughness_texture_dropdown_item",
                                                                        asset_library_state->material_modified);
                            if (dropdown_selected_index != selected_index) {
                                selected_index = dropdown_selected_index;
                                replace_contents(&selected_material->roughness_texture_name, texture_names[selected_index]);
                            }
                        } else {
                            do_text("Roughness");
                            selected_material->roughness = do_text_field_slider(selected_material->roughness,
                                                                                0.0f, 1.0f,
                                                                                editor_slider_theme,
                                                                                "material-roughness-slider",
                                                                                "material-roughness-slider",
                                                                                asset_library_state->material_modified);
                        }
                    
                        ui_y_pad(5.0f);
                    }
                }
            } ui_pop_widget(); // section

            UI_Theme footer_theme = {};
            footer_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
            footer_theme.layout_type = UI_LAYOUT_HORIZONTAL;

            ui_add_and_push_widget("", footer_theme);
            {
                UI_Theme push_right = {};
                push_right.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE };
                ui_add_widget("", push_right);

                UI_Button_Theme delete_theme = editor_button_danger_theme;
                delete_theme.size_type.x = UI_SIZE_ABSOLUTE;
                delete_theme.size.x = 120.0f;
                
                bool32 delete_pressed = do_text_button("Delete Material", delete_theme, "delete-material");

                if (delete_pressed) {
                    // find material id in list
                    int32 id_to_delete = selected_material->id;

                    for (int32 i = 0; i < num_materials_listed; i++) {
                        if (material_ids[i] == id_to_delete) {
                            if (num_materials_listed >= 1) {
                                asset_library_state->selected_material_id = material_ids[max(i - 1, 0)];
                            } else {
                                // we're deleting the last one
                                asset_library_state->selected_material_id = -1;
                            }

                            break;
                        }
                    }

                    selected_material = get_material(asset_library_state->selected_material_id);

                    delete_material(id_to_delete);
                    num_materials_listed = max(num_materials_listed--, 0);
                    asset_library_state->material_modified = true;
                }
            } ui_pop_widget(); // footer
        }
        ui_pop_widget();
    }
    ui_pop_widget(); // container

    ui_pop_widget(); // window

    // all the consumers of this have ran
    asset_library_state->material_modified = false;
}

void draw_level_box() {
    Editor_State *editor_state = &game_state->editor_state;

    UI_Theme column_theme = {};
    column_theme.size_type = { UI_SIZE_FILL_REMAINING, UI_SIZE_FIT_CHILDREN };
    column_theme.semantic_size = { 1.0f, 0.0f };
    column_theme.layout_type = UI_LAYOUT_VERTICAL;
    
    UI_Theme row_theme = {};
    row_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_FIT_CHILDREN };
    row_theme.semantic_size = { 1.0f, 0.0f };
    row_theme.layout_type = UI_LAYOUT_HORIZONTAL;
    //row_theme.position_type = theme.position_type;
    //row_theme.semantic_position = theme.position;
    //row_theme.background_color = theme.background_color;

    UI_Button_Theme theme = editor_button_theme;
    theme.size_type = { UI_SIZE_ABSOLUTE, UI_SIZE_ABSOLUTE };
    theme.size.x = 75.0f;
    
    ui_add_and_push_widget("", column_theme);
    {
        // this is so that we can reset the UI to new states, like the level name text field, when
        // we create a new level or load a different level
        bool32 just_changed_level = false;
        
        ui_add_and_push_widget("", row_theme);
        {
            bool32 new_level_clicked = do_text_button("New", theme, "new_level");
            if (new_level_clicked) {
                new_level(&game_state->level);
                reset_editor(editor_state);
                editor_state->is_new_level = true;
                just_changed_level = true;
            }
        
            ui_x_pad(1.0f);
        
            bool32 open_level_clicked = do_text_button("Open", theme, "open_level");
            if (open_level_clicked) {
                Marker m = begin_region();
                char *absolute_filename = (char *) region_push(PLATFORM_MAX_PATH);
        
                if (platform_open_file_dialog(absolute_filename,
                                              LEVEL_FILE_FILTER_TITLE, LEVEL_FILE_FILTER_TYPE,
                                              PLATFORM_MAX_PATH)) {
                    unload_level(&game_state->level);
                    bool32 result = read_and_load_level(&game_state->level, absolute_filename);
                    if (result) {
                        reset_editor(editor_state);
                        editor_state->is_new_level = false;
                        just_changed_level = true;
                    }
                }

                end_region(m);
            }

            
        }
        ui_pop_widget();

        ui_y_pad(5.0f);
        do_text("Level Name");
        ui_y_pad(1.0f);
        
        {
            UI_Text_Field_Theme field_theme = {};
            field_theme.background_color = editor_button_theme.background_color;
            field_theme.hot_background_color = editor_button_theme.hot_background_color;
            field_theme.active_background_color = editor_button_theme.active_background_color;
            field_theme.cursor_color = rgb_to_vec4(0, 255, 0);
            field_theme.font = default_font;
            field_theme.size_type = { UI_SIZE_PERCENTAGE, UI_SIZE_ABSOLUTE };
            field_theme.size = { 1.0f, editor_button_theme.size.y };

            UI_Text_Field_Result name_result = do_text_field(field_theme, game_state->level.name,
                                                             just_changed_level,
                                                             "level_name_text_field",
                                                             "level_name_text_field_text");
            replace_contents(&game_state->level.name, name_result.text);
        }

        ui_y_pad(5.0f);
        
        ui_add_and_push_widget("", row_theme);
        {
            bool32 save_level_clicked = do_text_button("Save", theme, "save_level");

            if (save_level_clicked) {
                Marker m = begin_region();
                
                if (editor_state->is_new_level) {
                    char *filename = (char *) region_push(PLATFORM_MAX_PATH);
                    bool32 has_filename = platform_open_save_file_dialog(filename,
                                                                         LEVEL_FILE_FILTER_TITLE, LEVEL_FILE_FILTER_TYPE,
                                                                         PLATFORM_MAX_PATH);

                    if (has_filename) {
                        export_level(&game_state->level, filename);
                        editor_state->is_new_level = false;
                
                        add_message(&game_state->message_manager, make_string(SAVE_SUCCESS_MESSAGE));
                    }
                } else {
                    export_level(&game_state->level, to_char_array(temp_region, game_state->level.filename));
                    add_message(&game_state->message_manager, make_string(SAVE_SUCCESS_MESSAGE));
                }

                end_region(m);
            }
            
            ui_x_pad(1.0f);

            bool32 save_as_level_clicked = do_text_button("Save As..", theme, "save_as");
            if (save_as_level_clicked) {
                assert(!is_empty(game_state->level.name));

                Marker m = begin_region();
                char *filename = (char *) region_push(PLATFORM_MAX_PATH);

                bool32 has_filename = platform_open_save_file_dialog(filename,
                                                                     LEVEL_FILE_FILTER_TITLE, LEVEL_FILE_FILTER_TYPE,
                                                                     PLATFORM_MAX_PATH);

                if (has_filename) {
                    export_level(&game_state->level, filename);
                    editor_state->is_new_level = false;
                    add_message(&game_state->message_manager, make_string(SAVE_SUCCESS_MESSAGE));
                }
        
                end_region(m);
            }
        }
        ui_pop_widget();
    }
    ui_pop_widget();
}

void draw_editor(Controller_State *controller_state) {
    Editor_State *editor_state = &game_state->editor_state;
    
    Entity *selected_entity = get_selected_entity(editor_state);

    Vec4 sidebar_background = DEFAULT_BOX_BACKGROUND;
            
    UI_Container_Theme sidebar_theme = {
        { 5.0f, 5.0f, 5.0f, 5.0f },
        sidebar_background,
        UI_POSITION_FLOAT,
        { render_state->display_output.width - 200.0f, 0.0f },
        { UI_SIZE_ABSOLUTE, UI_SIZE_FIT_CHILDREN },
        { 200.0f, 0.0f },
        UI_LAYOUT_VERTICAL
    };

    ui_push_existing_widget(ui_manager->main_layer);

    #if 1
    ui_push_container(sidebar_theme, "editor_sidebar");
    {
        bool32 toggle_show_wireframe_clicked = do_text_button(editor_state->show_wireframe ?
                                                              "Hide Wireframe" : "Show Wireframe",
                                                              editor_button_theme,
                                                              "toggle_wireframe");
    
        if (toggle_show_wireframe_clicked) {
            editor_state->show_wireframe = !editor_state->show_wireframe;
        }

        ui_y_pad(1.0f);
        
        Gizmo_State *gizmo_state = &editor_state->gizmo_state;
        bool32 toggle_global_clicked = do_text_button(gizmo_state->transform_mode == TRANSFORM_GLOBAL ?
                                                      "Use Local Transform" : "Use Global Transform",
                                                      editor_button_theme, "toggle_transform");
        if (toggle_global_clicked) {
            if (gizmo_state->transform_mode == TRANSFORM_GLOBAL) {
                gizmo_state->transform_mode = TRANSFORM_LOCAL;
            } else {
                gizmo_state->transform_mode = TRANSFORM_GLOBAL;
            }
        }

        ui_y_pad(20.0f);

        draw_level_box();

        ui_y_pad(20.0f);

        bool32 add_entity_clicked = do_text_button("Add Entity", editor_button_theme, "add_entity");
        if (add_entity_clicked) {
            Marker m = begin_region();
            Entity_Info info = {};
            info.flags = ENTITY_MESH | ENTITY_MATERIAL;
            info.transform = make_transform();
            info.mesh_name = make_string("cube");
            info.material_name = make_string("default_material");
            
            make_and_add_entity(&game_state->level, info);
            end_region(m);
        }

        ui_y_pad(10.0f);
        bool32 open_material_library_clicked = do_text_button("Material Library", editor_button_theme,
                                                              "open_material_library");

        UI_Window_Theme window_theme = DEFAULT_WINDOW_THEME;
        if (open_material_library_clicked) {
            if (!editor_state->is_material_library_window_open) {
                reset_asset_library_state();
            }
            editor_state->is_material_library_window_open = !editor_state->is_material_library_window_open;
        }

        if (editor_state->is_material_library_window_open) {
            draw_asset_library();
        }
    }
    ui_pop_widget();
    #endif

    if (editor_state->selected_entity_id >= 0) {
        // if we changed entities this frame, then hide it so it can reset and show up next frame
        draw_entity_box_2(editor_state->selected_entity_modified);
    }
    
#if 0
    if (selected_entity) {
        draw_entity_box(editor_state, ui_manager, controller_state, selected_entity);

        if (editor_state->open_window_flags & MATERIAL_LIBRARY_WINDOW) {
            draw_material_library(editor_state, ui_manager, controller_state, render_state, selected_entity);
        } else if (editor_state->open_window_flags & TEXTURE_LIBRARY_WINDOW) {
            if (has_material_field(selected_entity)) {
                draw_texture_library(editor_state, ui_manager, controller_state, render_state,
                                     get_material_id(selected_entity));
            }
        } else if (editor_state->open_window_flags & MESH_LIBRARY_WINDOW) {
            draw_mesh_library(editor_state, ui_manager, controller_state, render_state, selected_entity);
        }
    } else {
        reset_entity_editors(editor_state);
    }



    real32 y = 0.0f;
    real32 button_gap = 1.0f;
    real32 sidebar_button_width = 200.0f;

    int32 font_id;
    Font font = get_font(&editor_state->asset_manager, Editor_Constants::editor_font_name, &font_id);

    Gizmo_State *gizmo_state = &editor_state->gizmo_state;

    real32 button_height = 25.0f;
    // wireframe toggle
    real32 wireframe_button_width = sidebar_button_width;
    bool32 toggle_show_wireframe_clicked = do_text_button(render_state->display_output.width - sidebar_button_width,
                                                          y,
                                                          wireframe_button_width, button_height,
                                                          default_text_button_style, default_text_style,
                                                          editor_state->show_wireframe ?
                                                          "Hide Wireframe" : "Show Wireframe",
                                                          font_id, "toggle_wireframe");
    if (toggle_show_wireframe_clicked) {
        editor_state->show_wireframe = !editor_state->show_wireframe;
    }
    y += button_height + button_gap;

    // transform mode toggle
    bool32 toggle_global_clicked = do_text_button(render_state->display_output.width - sidebar_button_width, y,
                                                  sidebar_button_width, button_height,
                                                  default_text_button_style, default_text_style,
                                                  gizmo_state->transform_mode == TRANSFORM_GLOBAL ?
                                                  "Use Local Transform" : "Use Global Transform",
                                                  font_id, "toggle_transform");
    if (toggle_global_clicked) {
        if (gizmo_state->transform_mode == TRANSFORM_GLOBAL) {
            gizmo_state->transform_mode = TRANSFORM_LOCAL;
        } else {
            gizmo_state->transform_mode = TRANSFORM_GLOBAL;
        }
    }
    y += button_height + button_gap;

    y += 5;
    draw_level_box(&game_state->ui_manager, editor_state,
                   controller_state,
                   render_state->display_output.width - 198.0f - 1.0f, y);

    y += 120.0f;

    bool32 add_normal_entity_clicked = do_text_button(render_state->display_output.width - sidebar_button_width, y,
                                                      sidebar_button_width, button_height,
                                                      default_text_button_style, default_text_style,
                                                      "Add Normal Entity",
                                                      font_id, "add_entity");
    y += button_height + button_gap;
    if (add_normal_entity_clicked) {
        do_add_normal_entity(editor_state);
    }

    bool32 add_point_light_entity_clicked = do_text_button(render_state->display_output.width - sidebar_button_width, y,
                                                           sidebar_button_width, button_height,
                                                           default_text_button_style, default_text_style,
                                                           "Add Point Light Entity",
                                                           font_id, "add_point_light_entity");
    y += button_height + button_gap;
    if (add_point_light_entity_clicked) {
        do_add_point_light_entity(editor_state);
    }

    bool32 toggle_colliders_clicked = do_text_button(render_state->display_output.width - sidebar_button_width, y,
                                                     sidebar_button_width, button_height,
                                                     default_text_button_style, default_text_style,
                                                     editor_state->show_colliders ?
                                                     "Hide Colliders" : "Show Colliders",
                                                     font_id, "toggle_show_colliders");

    if (toggle_colliders_clicked) {
        editor_state->show_colliders = !editor_state->show_colliders;
    }

    if (!editor_state->is_new_level) {
        char *filename_buf = to_char_array((Allocator *) &memory.frame_arena, editor_state->level_filename);
        char *buf = string_format((Allocator *) &memory.frame_arena, PLATFORM_MAX_PATH + 32,
                                  "current level: %s", filename_buf);
        do_text(ui_manager,
                5.0f, render_state->display_output.height - 9.0f,
                buf, font_id, default_text_style, "editor_current_level_filename");
    }

    y += button_height + button_height;

    int32 num_history_entries = history_get_num_entries(&editor_state->history);
    bool32 undo_clicked = do_text_button(render_state->display_output.width - sidebar_button_width, y,
                                         0.5f * sidebar_button_width - 1, button_height,
                                         default_text_button_style, default_text_style,
                                         "Undo",
                                         font_id,
                                         editor_state->history.num_undone == num_history_entries,
                                         "editor_undo");
    if (undo_clicked) {
        history_undo(editor_state);
    }

    bool32 redo_clicked = do_text_button(render_state->display_output.width - 0.5f * sidebar_button_width, y,
                                         0.5f * sidebar_button_width, button_height,
                                         default_text_button_style, default_text_style,
                                         "Redo",
                                         font_id,
                                         num_history_entries == 0 || editor_state->history.num_undone == 0,
                                         "editor_redo");
    if (redo_clicked) {
        history_redo(editor_state);
    }
    #endif

    #if 0
    Allocator *frame_allocator = (Allocator *) &memory.frame_arena;
    char *buf = string_format(frame_allocator, 64, "editor history heap used: %d",
                              editor_state->history_heap.used);
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 90.0f,
            buf, font_id, default_text_style, "editor history heap used");

    buf = string_format(frame_allocator, 64, "editor entity heap used: %d",
                        editor_state->entity_heap.used);
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 72.0f,
            buf, font_id, default_text_style, "editor entity heap used");
    #endif

#if 0
    char *buf = string_format(frame_allocator, 64, "heap size: %d", ui_manager->heap_pointer->size);
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 62.0f,
            buf, editor_font_name, default_text_style, "heap size");

    buf = string_format((Allocator *) &memory.frame_arena, 64, "heap used: %d", ui_manager->heap_pointer->used);
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 48.0f,
            buf, editor_font_name, default_text_style, "heap used");
#endif

    
#if 0
    buf = string_format(frame_allocator, 64, "num_undone: %d", editor_state->history.num_undone);
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 129.0f,
            buf, editor_font_name, default_text_style, "num undone");

    buf = string_format((Allocator *) &memory.frame_arena, 64, "num_history_entries: %d", num_history_entries);
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 143.0f,
            buf, editor_font_name, default_text_style, "num history entries");

    Heap_Allocator *history_heap = &memory.editor_history_heap;
    String_Buffer history_buf = make_string_buffer((Allocator *) &memory.frame_arena, 512);
    append_string(&history_buf, "[ ");

    Heap_Block *block = history_heap->first_block;
    while (block) {
        Marker m = begin_region();
        char *whatever = string_format((Allocator *) &memory.global_stack, 16, "| %d | ", block->size);
        append_string(&history_buf, whatever);
        end_region(m);

        block = block->next;
    }

    append_string(&history_buf, "]");

    buf = to_char_array((Allocator *) &memory.frame_arena, history_buf);
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 114.0f,
            buf, editor_font_name, default_text_style, "editor history");
#endif

    ui_pop_widget();
}

void editor_post_update() {
    Editor_State *editor_state = &game_state->editor_state;

    // note that the reason why this can be here is because entity gizmo operations are done before the UI.
    // if it was UI -> gizmo -> editor_post_update(), the UI would never get updated by gizmo operations
    // because this would always reset it. see draw_asset_library() for a case where we couldn't put the
    // modified flag resetting here.
    editor_state->selected_entity_modified = false;
}
