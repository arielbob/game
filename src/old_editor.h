#ifndef EDITOR_H
#define EDITOR_H

#include "editor_history.h"

#define LEVEL_FILE_FILTER_TITLE "Levels (*.level)"
#define LEVEL_FILE_FILTER_TYPE "level"
#define SAVE_SUCCESS_MESSAGE "Level saved!"

//enum class Editor_Color_Picker { NONE = 0, MATERIAL_COLOR_OVERRIDE, POINT_LIGHT_COLOR };

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

struct Gizmo {
    Transform transform;
    int32 arrow_mesh_id;
    int32 ring_mesh_id;
    int32 sphere_mesh_id;
    int32 cube_mesh_id;
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

namespace Editor_Constants {
    real32 point_light_side_length = 0.4f;
    real32 camera_speed = 19.0f;
    Vec4 row_color = make_vec4(0.12f, 0.12f, 0.12f, 0.9f);
    Vec4 darkened_row_color = make_vec4(0.0f, 0.0f, 0.0f, 0.9f);
    Vec4 border_color = make_vec4(0.3f, 0.3f, 0.3f, 1.0f);
    real32 hsv_picker_width = 200.0f;
    real32 hsv_picker_height = 200.0f;
    real32 hue_slider_width = 20.0f;
    real32 small_padding_x = 6.0f;
    real32 small_padding_y = 6.0f;
    real32 x_nested_offset = 10.0f;
    real32 color_picker_width = (small_padding_x +
                                 hsv_picker_width + small_padding_x +
                                 hue_slider_width + small_padding_x);
    real32 color_picker_height = 200.0f + small_padding_y*2;

    char disallowed_chars[] = {'{', '}', '"'};
    int32 num_disallowed_chars = array_length(disallowed_chars);

    UI_Color_Picker_Style color_picker_style = {
        color_picker_width, color_picker_height,
        hsv_picker_width, hsv_picker_height,
        hue_slider_width,
        small_padding_x, small_padding_y,
        row_color
    };
};

#define MATERIAL_LIBRARY_WINDOW 0x1
#define TEXTURE_LIBRARY_WINDOW  0x2
#define MESH_LIBRARY_WINDOW     0x4

struct Editor_State {
    Editor_History history;

    Transform_Mode transform_mode;

    bool32 use_freecam;
    
    int32 selected_entity_id;
    Entity_Type selected_entity_type;

    int32 last_selected_entity_id;
    Entity_Type last_selected_entity_type;

    Entity *old_entity;
    Material old_material;

    Gizmo gizmo;
    Gizmo_Handle hovered_gizmo_handle;
    Gizmo_Handle selected_gizmo_handle;
    //Vec3 gizmo_initial_hit;
    //Vec3 gizmo_local_initial_hit;
    Vec3 local_initial_gizmo_hit;
    Vec3 global_initial_gizmo_hit;

    Vec3 gizmo_transform_axis;
    //Vec3 last_gizmo_transform_point;

    bool32 show_wireframe;
    bool32 show_colliders;
    uint32 open_window_flags;
    Mesh_Type mesh_library_filter;
    bool32 editing_selected_entity_material;
    bool32 editing_selected_entity_texture;
    bool32 editing_selected_entity_mesh;

    bool32 is_new_level;
    String_Buffer current_level_filename;

    UI_id color_picker_parent;
};

#endif
