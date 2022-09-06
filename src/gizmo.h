#ifndef GIZMO_H
#define GIZMO_H

enum Gizmo_Handle {
    GIZMO_HANDLE_NONE,
    GIZMO_TRANSLATE_X,
    GIZMO_TRANSLATE_Y,
    GIZMO_TRANSLATE_Z,
    GIZMO_SCALE_X,
    GIZMO_SCALE_Y,
    GIZMO_SCALE_Z,
    GIZMO_ROTATE_X,
    GIZMO_ROTATE_Y,
    GIZMO_ROTATE_Z
};

enum Transform_Mode {
    TRANSFORM_GLOBAL,
    TRANSFORM_LOCAL
};

namespace Gizmo_Constants {
    Vec4 default_x_handle_color = make_vec4(x_axis, 1.0f);
    Vec4 default_y_handle_color = make_vec4(y_axis, 1.0f);
    Vec4 default_z_handle_color = make_vec4(z_axis, 1.0f);
    Vec4 x_handle_hover = make_vec4(1.0f, 0.8f, 0.8f, 1.0f);
    Vec4 y_handle_hover = make_vec4(0.8f, 1.0f, 0.8f, 1.0f);
    Vec4 z_handle_hover = make_vec4(0.8f, 0.8f, 1.0f, 1.0f);
    Transform scale_handle_transform = make_transform(make_vec3(0.8f, 0.0f, 0.0f),
                                                      make_quaternion(),
                                                      make_vec3(0.3f, 0.1f, 0.1f));
    Mat4 scale_handle_model_matrix = get_model_matrix(scale_handle_transform);
};

struct Gizmo_State {
    Transform_Mode transform_mode;
    Transform transform;
    String arrow_mesh_name;
    String ring_mesh_name;
    String sphere_mesh_name;
    String cube_mesh_name;

    Gizmo_Handle hovered_gizmo_handle;
    Gizmo_Handle selected_gizmo_handle;

    Vec3 local_initial_gizmo_hit;
    Vec3 global_initial_gizmo_hit;

    Vec3 gizmo_transform_axis;

    Transform original_entity_transform;
};

#endif
