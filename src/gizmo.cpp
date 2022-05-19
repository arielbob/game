#include "gizmo.h"

bool32 is_translation(Gizmo_Handle gizmo_axis) {
    return (gizmo_axis == GIZMO_TRANSLATE_X ||
            gizmo_axis == GIZMO_TRANSLATE_Y ||
            gizmo_axis == GIZMO_TRANSLATE_Z);
}

bool32 is_scale(Gizmo_Handle gizmo_axis) {
    return (gizmo_axis == GIZMO_SCALE_X ||
            gizmo_axis == GIZMO_SCALE_Y ||
            gizmo_axis == GIZMO_SCALE_Z);
}

bool32 is_rotation(Gizmo_Handle gizmo_axis) {
    return (gizmo_axis == GIZMO_ROTATE_X ||
            gizmo_axis == GIZMO_ROTATE_Y ||
            gizmo_axis == GIZMO_ROTATE_Z);
}

Vec3 get_gizmo_hit(Gizmo_State *gizmo_state, Gizmo_Handle picked_handle, real32 pick_gizmo_t,
                   Ray cursor_ray, Vec3 transform_axis) {
    Vec3 gizmo_hit;

    if (is_rotation(picked_handle)) {
        real32 t;
        Plane plane = { dot(gizmo_state->transform.position, transform_axis), transform_axis };
        bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);
        if (intersects_plane) {
            gizmo_hit = cursor_ray.origin + t*cursor_ray.direction;
        } else {
            gizmo_hit = cursor_ray.origin + pick_gizmo_t*cursor_ray.direction;;
        }
    } else {
        gizmo_hit = cursor_ray.origin + pick_gizmo_t*cursor_ray.direction;
    }

    return gizmo_hit;
}

Vec3 get_gizmo_transform_axis(Transform_Mode transform_mode, Gizmo_Handle gizmo_handle, Transform transform) {
    assert(gizmo_handle != GIZMO_HANDLE_NONE);

    uint32 axis;

    if (gizmo_handle == GIZMO_TRANSLATE_X ||
        gizmo_handle == GIZMO_SCALE_X ||
        gizmo_handle == GIZMO_ROTATE_X) {
        // x
        axis = 0;
    } else if (gizmo_handle == GIZMO_TRANSLATE_Y ||
               gizmo_handle == GIZMO_SCALE_Y ||
               gizmo_handle == GIZMO_ROTATE_Y) {
        // y
        axis = 1;
    } else {
        // z
        axis = 2;
    }
     
    if (transform_mode == TRANSFORM_GLOBAL) {
        if (axis == 0) {
            return x_axis;
        } else if (axis == 1) {
            return y_axis;
        } else {
            return z_axis;
        }
    } else {
        // local transform

        // TODO: maybe cache this model matrix?
        Mat4 entity_model_matrix = get_model_matrix(transform);

        if (axis == 0) {
            return normalize(truncate_v4_to_v3(entity_model_matrix.col1));
        } else if (axis == 1) {
            return  normalize(truncate_v4_to_v3(entity_model_matrix.col2));
        } else {
            return normalize(truncate_v4_to_v3(entity_model_matrix.col3));
        }
    }
}
