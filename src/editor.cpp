#include "editor.h"
#include "game.h"

void draw_entity_box(Memory *memory, Game_State *game_state, Controller_State *controller_state, Entity *entity) {
    UI_Manager *ui_manager = &game_state->ui_manager;

    real32 box_x = 5.0f;
    real32 box_y = 50.0f;

    real32 box_padding_x = 5.0f;
    real32 box_padding_y = 5.0f;

    UI_Box_Style box_style = { 350.0f, 265.0f, make_vec4(0.1f, 0.1f, 0.1f, 0.9f) };
    do_box(ui_manager, controller_state, box_x, box_y, box_style, "entity_properties_box");

    char *mesh_name = game_state->meshes[entity->mesh_index].name;
    Transform transform = entity->transform;

    UI_Text_Style text_style = {
        make_vec3(1.0f, 1.0f, 1.0f),
        true,
        make_vec3(0.0f, 0.0f, 0.0f)
    };

    UI_Line_Style line_style = {
        make_vec4(1.0f, 1.0f, 1.0f, 1.0f),
        1.0f,
    };

    real32 offset_x = box_x + box_padding_x;
    real32 offset_y = box_y + box_padding_y;
  
    real32 column_offset = 200.0f;
    real32 mini_column_offset = 20.0f;

    real32 font_height = 18.0f;
    real32 row_offset = font_height + 5.0f;

    char *font_name = "courier18";
    char *font_name_bold = "courier18b";

    Allocator *allocator = (Allocator *) &memory->frame_arena;
    char *buf;
    
    do_text(ui_manager, offset_x, offset_y + font_height, "Mesh Name", font_name_bold, text_style, "entity_name_title");
    do_text(ui_manager, offset_x + column_offset, offset_y + font_height, mesh_name, font_name, text_style, "entity_name");
    offset_y += row_offset;

    do_text(ui_manager, offset_x, offset_y + font_height, "Position", font_name_bold, text_style, "position_title");
    buf = string_format(allocator, 16, "%f", transform.position.x);
    do_text(ui_manager, offset_x + column_offset, offset_y + font_height,
            "x", font_name_bold, text_style, "position.x");
    do_text(ui_manager, offset_x + mini_column_offset + column_offset, offset_y + font_height,
            buf, font_name, text_style, "position.x");
    offset_y += font_height;

    buf = string_format(allocator, 16, "%f", transform.position.y);
    do_text(ui_manager, offset_x + column_offset, offset_y + font_height,
            "y", font_name_bold, text_style, "position.x");
    do_text(ui_manager, offset_x + mini_column_offset + column_offset, offset_y + font_height,
            buf, font_name, text_style, "position.y");
    offset_y += font_height;

    buf = string_format(allocator, 16, "%f", transform.position.z);
    do_text(ui_manager, offset_x + column_offset, offset_y + font_height,
            "z", font_name_bold, text_style, "position.x");
    do_text(ui_manager, offset_x + mini_column_offset + column_offset, offset_y + font_height,
            buf, font_name, text_style, "position.z");
    offset_y += row_offset;

    do_text(ui_manager, offset_x, offset_y + font_height, "Rotation", font_name_bold, text_style, "rotation_title");
    buf = string_format(allocator, 16, "%f", transform.rotation.w);
    do_text(ui_manager, offset_x + column_offset, offset_y + font_height,
            "w", font_name_bold, text_style, "position.x");
    do_text(ui_manager, offset_x + mini_column_offset + column_offset, offset_y + font_height,
            buf, font_name, text_style, "rotation.w");
    offset_y += font_height;

    buf = string_format(allocator, 16, "%f", transform.rotation.v.x);
    do_text(ui_manager, offset_x + column_offset, offset_y + font_height,
            "x", font_name_bold, text_style, "position.x");
    do_text(ui_manager, offset_x + mini_column_offset + column_offset, offset_y + font_height,
            buf, font_name, text_style, "rotation.v.x");
    offset_y += font_height;

    buf = string_format(allocator, 16, "%f", transform.rotation.v.y);
    do_text(ui_manager, offset_x + column_offset, offset_y + font_height,
            "y", font_name_bold, text_style, "position.x");
    do_text(ui_manager, offset_x + mini_column_offset + column_offset, offset_y + font_height,
            buf, font_name, text_style, "rotation.v.y");
    offset_y += font_height;

    buf = string_format(allocator, 16, "%f", transform.rotation.v.z);
    do_text(ui_manager, offset_x + column_offset, offset_y + font_height,
            "z", font_name_bold, text_style, "rotation.x");
    do_text(ui_manager, offset_x + mini_column_offset + column_offset, offset_y + font_height,
            buf, font_name, text_style, "rotation.v.z");
    offset_y += row_offset;

    do_text(ui_manager, offset_x, offset_y + font_height, "Scale", font_name_bold, text_style, "scale_title");
    buf = string_format(allocator, 16, "%f", transform.scale.x);
    do_text(ui_manager, offset_x + column_offset, offset_y + font_height,
            "x", font_name_bold, text_style, "scale.x");
    do_text(ui_manager, offset_x + mini_column_offset + column_offset, offset_y + font_height,
            buf, font_name, text_style, "scale.x");
    offset_y += font_height;

    buf = string_format(allocator, 16, "%f", transform.scale.y);
    do_text(ui_manager, offset_x + column_offset, offset_y + font_height,
            "y", font_name_bold, text_style, "scale.y");
    do_text(ui_manager, offset_x + mini_column_offset + column_offset, offset_y + font_height,
            buf, font_name, text_style, "scale.y");
    offset_y += font_height;

    buf = string_format(allocator, 16, "%f", transform.scale.z);
    do_text(ui_manager, offset_x + column_offset, offset_y + font_height,
            "z", font_name_bold, text_style, "scale.z");
    do_text(ui_manager, offset_x + mini_column_offset + column_offset, offset_y + font_height,
            buf, font_name, text_style, "scale.z");
    offset_y += row_offset;

}

int32 ray_intersects_mesh(Ray ray, Mesh mesh, Transform transform, real32 *t_result) {
    Mat4 object_to_world = get_model_matrix(transform);
    Mat4 world_to_object = inverse(object_to_world);

    // instead of transforming every vertex, we just transform the world-space ray to object-space by
    // multiplying the origin and the direction of the ray by the inverse of the entity's model matrix.
    // note that we must zero-out the w component of the ray direction when doing this multiplication so that
    // we ignore the translation of the model matrix.
    Vec3 object_space_ray_origin = truncate_v4_to_v3(world_to_object * make_vec4(ray.origin, 1.0f));
    Vec3 object_space_ray_direction = normalize(truncate_v4_to_v3(world_to_object *
                                                                  make_vec4(ray.direction, 0.0f)));
    Ray object_space_ray = { object_space_ray_origin, object_space_ray_direction };

    uint32 *indices = mesh.indices;

    // this might be very slow
    real32 t_min = FLT_MAX;
    bool32 hit = false;
    for (int32 i = 0; i < (int32) mesh.num_triangles; i++) {
        Vec3 triangle[3];
        triangle[0] = get_vertex_from_index(&mesh, indices[i * 3]);
        triangle[1] = get_vertex_from_index(&mesh, indices[i * 3 + 1]);
        triangle[2] = get_vertex_from_index(&mesh, indices[i * 3 + 2]);
        real32 t;

        // TODO: we might be able to pass t_min into this test to early-out before we check if a hit point
        //       is within the triangle, but after we've hit the plane
        if (ray_intersects_triangle(object_space_ray, triangle, &t)) {
            t_min = min(t, t_min);
            hit = true;
        }
    }

    if (hit) {
        // convert the t_min on the object space ray to a t_min on the world space ray
        Vec3 object_space_hit_point = object_space_ray_origin + object_space_ray_direction * t_min;
        Vec3 world_space_hit_point = truncate_v4_to_v3(object_to_world * make_vec4(object_space_hit_point, 1.0f));
        real32 world_space_t_min = dot(world_space_hit_point - ray.origin, ray.direction);
        *t_result = world_space_t_min;
    }

    return hit;
}

// TODO: optimize this by checking against AABB before checking against triangles
int32 pick_entity(Game_State *game_state, Ray cursor_ray, Entity *entity_result, int32 *index_result) {
    Editor_State *editor_state = &game_state->editor_state;

    Mesh *meshes = game_state->meshes;
    Normal_Entity *entities = game_state->entities;
    Point_Light_Entity *point_lights = game_state->point_lights;

    Entity *picked_entity = NULL;
    int32 entity_index = -1;

    real32 t_min = FLT_MAX;
    for (int32 i = 0; i < game_state->num_entities; i++) {
        real32 t;
        Normal_Entity *entity = &entities[i];
        Mesh mesh = meshes[entity->mesh_index];
        if (ray_intersects_mesh(cursor_ray, mesh, entity->transform, &t) && (t < t_min)) {
            t_min = t;
            entity_index = i;
            picked_entity = (Entity *) entity;
        }
    }

    for (int32 i = 0; i < game_state->num_point_lights; i++) {
        real32 t;
        Point_Light_Entity *entity = &point_lights[i];
        Mesh mesh = meshes[entity->mesh_index];
        if (ray_intersects_mesh(cursor_ray, mesh, entity->transform, &t) && (t < t_min)) {
            t_min = t;
            entity_index = i;
            picked_entity = (Entity *) entity;
        }
    }

    if (entity_index >= 0) {
        *entity_result = *picked_entity;
        *index_result = entity_index;
        return true;
    }

    return false;
}

bool32 is_translation(Gizmo_Handle gizmo_axis) {
    return (gizmo_axis == GIZMO_TRANSLATE_X ||
            gizmo_axis == GIZMO_TRANSLATE_Y ||
            gizmo_axis == GIZMO_TRANSLATE_Z);
}

bool32 is_rotation(Gizmo_Handle gizmo_axis) {
    return (gizmo_axis == GIZMO_ROTATE_X ||
            gizmo_axis == GIZMO_ROTATE_Y ||
            gizmo_axis == GIZMO_ROTATE_Z);
}

Gizmo_Handle pick_gizmo(Game_State *game_state, Ray cursor_ray,
                        Vec3 *gizmo_initial_hit, Vec3 *gizmo_transform_axis) {
    Editor_State *editor_state = &game_state->editor_state;
    Entity *entity = get_selected_entity(game_state);
    Gizmo gizmo = editor_state->gizmo;

    Transform_Mode transform_mode = editor_state->transform_mode;

    Mat4 entity_model_matrix = get_model_matrix(entity->transform);

    Transform x_transform, y_transform, z_transform;
    Vec3 transform_x_axis, transform_y_axis, transform_z_axis;
    // TODO: maybe add some scale here? for a bit of extra space to click on the gizmo.
    if (transform_mode == TRANSFORM_GLOBAL) {
        x_transform = gizmo.transform;
        x_transform.rotation = make_quaternion();
        y_transform = gizmo.transform;
        y_transform.rotation = make_quaternion(90.0f, z_axis);
        z_transform = gizmo.transform;
        z_transform.rotation = make_quaternion(-90.0f, y_axis);

        transform_x_axis = x_axis;
        transform_y_axis = y_axis;
        transform_z_axis = z_axis;
    } else {
        // local transform
        x_transform = gizmo.transform;
        y_transform = gizmo.transform;
        y_transform.rotation = gizmo.transform.rotation*make_quaternion(90.0f, z_axis);
        z_transform = gizmo.transform;
        z_transform.rotation = gizmo.transform.rotation*make_quaternion(-90.0f, y_axis);

        transform_x_axis = normalize(truncate_v4_to_v3(entity_model_matrix.col1));
        transform_y_axis = normalize(truncate_v4_to_v3(entity_model_matrix.col2));
        transform_z_axis = normalize(truncate_v4_to_v3(entity_model_matrix.col3));
    }

    Gizmo_Handle picked_handle = GIZMO_HANDLE_NONE;

    Transform gizmo_handle_transforms[3] = { x_transform, y_transform, z_transform };
    Vec3 transform_axes[3] = { transform_x_axis, transform_y_axis, transform_z_axis };
    Vec3 selected_transform_axis = transform_x_axis;

    // check ray against translation arrows
    Gizmo_Handle gizmo_translation_handles[3] = { GIZMO_TRANSLATE_X, GIZMO_TRANSLATE_Y, GIZMO_TRANSLATE_Z };
    int32 gizmo_arrow_mesh_index = get_mesh_index(game_state, gizmo.arrow_mesh_name);
    assert(gizmo_arrow_mesh_index >= 0);
    Mesh arrow_mesh = game_state->meshes[gizmo_arrow_mesh_index];

    real32 t_min = FLT_MAX;
    for (int32 i = 0; i < 3; i++) {
        Transform gizmo_handle_transform = gizmo_handle_transforms[i];
        real32 t;
        if (ray_intersects_mesh(cursor_ray, arrow_mesh, gizmo_handle_transform, &t) && (t < t_min)) {
            t_min = t;
            picked_handle = gizmo_translation_handles[i];
            selected_transform_axis = transform_axes[i];
        }
    }

    // check ray against rotation rings
    Gizmo_Handle gizmo_rotation_handles[3] = { GIZMO_ROTATE_X, GIZMO_ROTATE_Y, GIZMO_ROTATE_Z };
    int32 gizmo_ring_mesh_index = get_mesh_index(game_state, gizmo.ring_mesh_name);
    assert(gizmo_ring_mesh_index >= 0);
    Mesh ring_mesh = game_state->meshes[gizmo_ring_mesh_index];

    for (int32 i = 0; i < 3; i++) {
        Transform gizmo_handle_transform = gizmo_handle_transforms[i];
        real32 t;
        if (ray_intersects_mesh(cursor_ray, ring_mesh, gizmo_handle_transform, &t) && (t < t_min)) {
            t_min = t;
            picked_handle = gizmo_rotation_handles[i];
            selected_transform_axis = transform_axes[i];
        }
    }

    if (picked_handle) {
        if (is_rotation(picked_handle)) {
            real32 t;
            Plane plane = { dot(gizmo.transform.position, selected_transform_axis),
                            selected_transform_axis };
            bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);
            if (intersects_plane) {
                *gizmo_initial_hit = cursor_ray.origin + t*cursor_ray.direction;
            } else {
                *gizmo_initial_hit = cursor_ray.origin + t_min*cursor_ray.direction;;
            }
        } else {
            *gizmo_initial_hit = cursor_ray.origin + t_min*cursor_ray.direction;
        }

        *gizmo_transform_axis = selected_transform_axis;
    }

    return picked_handle;
}

Vec3 do_gizmo_translation(Camera *camera, Editor_State *editor_state, Ray cursor_ray) {
    real32 t;
    Vec3 camera_forward = camera->current_basis.forward;
    Ray transform_ray = make_ray(editor_state->gizmo_initial_hit,
                                 editor_state->gizmo_transform_axis);
    Plane plane = get_plane_containing_ray(transform_ray, camera_forward);
    bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);

    Vec3 delta_result = make_vec3();

    // this will always intersect unless your FOV is >= 180 degrees
    if (intersects_plane) {
        Vec3 intersect_point = cursor_ray.origin + t*cursor_ray.direction;
        real32 delta_length = dot(intersect_point - editor_state->last_gizmo_transform_point,
                                  editor_state->gizmo_transform_axis);
        Vec3 delta = editor_state->gizmo_transform_axis * delta_length;
        editor_state->last_gizmo_transform_point += delta;
        delta_result = delta;
    }

    return delta_result;
}

Quaternion do_gizmo_rotation(Camera *camera, Editor_State *editor_state, Ray cursor_ray) {
    real32 t;
    Vec3 camera_forward = camera->current_basis.forward;
    Vec3 center = editor_state->gizmo.transform.position;
    Plane plane = { dot(center, editor_state->gizmo_transform_axis),
                    editor_state->gizmo_transform_axis };
    bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);

    Quaternion delta_result = make_quaternion();

    // this will always intersect unless your FOV is >= 180 degrees
    if (intersects_plane) {
        Vec3 intersect_point = cursor_ray.origin + t*cursor_ray.direction;

        Vec3 center_to_intersect_point = intersect_point - center;
        Vec3 center_to_last_intersect_point = editor_state->last_gizmo_transform_point - center;

        if (are_collinear(center_to_last_intersect_point, center_to_intersect_point)) {
            return delta_result;
        }

        Vec3 out_vector = cross(editor_state->gizmo_transform_axis, center_to_last_intersect_point);
        
        real32 sign = 1.0f;
        if (dot(center_to_intersect_point, out_vector) < 0.0f) sign = -1.0f;

        real32 a = distance(center_to_intersect_point);
        real32 b = distance(center_to_last_intersect_point);
        real32 c = distance(intersect_point - editor_state->last_gizmo_transform_point);

        real32 angle_delta_degrees = sign * cosine_law_degrees(a, b, c);
        Quaternion delta = make_quaternion(angle_delta_degrees, editor_state->gizmo_transform_axis);

        editor_state->last_gizmo_transform_point = intersect_point;
        delta_result = delta;
    }

    return delta_result;
}