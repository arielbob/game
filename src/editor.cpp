#include "editor.h"
#include "game.h"

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
        *t_result = t_min;
    }

    return hit;
}

// TODO: optimize this by checking against AABB before checking against triangles
int32 pick_entity(Game_State *game_state, Ray cursor_ray) {
    Editor_State *editor_state = &game_state->editor_state;

    Mesh *meshes = game_state->meshes;
    Entity *entities = game_state->entities;

    int32 entity_index = -1;

    real32 t_min = FLT_MAX;
    for (int32 i = 0; i < game_state->num_entities; i++) {
        real32 t;
        Entity entity = entities[i];
        Mesh mesh = meshes[entity.mesh_index];
        if (ray_intersects_mesh(cursor_ray, mesh, entity.transform, &t) && (t < t_min)) {
            t_min = t;
            entity_index = i;
        }
    }

    return entity_index;
}

Gizmo_Axis pick_gizmo(Game_State *game_state, Ray cursor_ray,
                Vec3 *gizmo_initial_hit, Vec3 *gizmo_transform_axis) {
    Editor_State *editor_state = &game_state->editor_state;
    Entity entity = game_state->entities[editor_state->selected_entity_index];
    Gizmo gizmo = editor_state->gizmo;

    Transform_Mode transform_mode = editor_state->transform_mode;

    Mat4 entity_model_matrix = get_model_matrix(entity.transform);

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

    Gizmo_Axis picked_axis = GIZMO_AXIS_NONE;
    Gizmo_Axis gizmo_axes[3] = { GIZMO_TRANSLATE_X, GIZMO_TRANSLATE_Y, GIZMO_TRANSLATE_Z };
    Transform gizmo_translate_axes_transforms[3] = { x_transform, y_transform, z_transform };
    Vec3 transform_axes[3] = { transform_x_axis, transform_y_axis, transform_z_axis };
    Vec3 selected_transform_axis = transform_x_axis;

    int32 gizmo_arrow_mesh_index = get_mesh_index(game_state, gizmo.arrow_mesh_name);
    assert(gizmo_arrow_mesh_index >= 0);
    Mesh arrow_mesh = game_state->meshes[gizmo_arrow_mesh_index];

    real32 t_min = FLT_MAX;
    for (int32 i = 0; i < 3; i++) {
        Transform gizmo_axis_transform = gizmo_translate_axes_transforms[i];
        real32 t;
        if (ray_intersects_mesh(cursor_ray, arrow_mesh, gizmo_axis_transform, &t) && (t < t_min)) {
            t_min = t;
            picked_axis = gizmo_axes[i];
            selected_transform_axis = transform_axes[i];
        }
    }

    if (picked_axis) {
        *gizmo_initial_hit = cursor_ray.origin + t_min * cursor_ray.direction;
        *gizmo_transform_axis = selected_transform_axis;
    }

    return picked_axis;
}

// TODO: clean up the unused variables in editor_state (this works now)
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
        editor_state->gizmo.transform.position += delta;
        editor_state->last_gizmo_transform_point += delta;
        delta_result = delta;
    }

    return delta_result;
}
