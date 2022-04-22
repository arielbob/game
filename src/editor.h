#ifndef EDITOR_H
#define EDITOR_H

#define LEVEL_FILE_FILTER_TITLE "Levels (*.level)"
#define LEVEL_FILE_FILTER_TYPE "level"

enum class Editor_Color_Picker { NONE = 0, MATERIAL_COLOR_OVERRIDE, POINT_LIGHT_COLOR };

enum Gizmo_Handle {
    GIZMO_HANDLE_NONE,
    GIZMO_TRANSLATE_X,
    GIZMO_TRANSLATE_Y,
    GIZMO_TRANSLATE_Z,
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
};

namespace Editor_Constants {
    Vec4 row_color = make_vec4(0.1f, 0.1f, 0.1f, 0.9f);
    Vec4 border_color = make_vec4(0.3f, 0.3f, 0.3f, 1.0f);
    real32 hsv_picker_width = 200.0f;
    real32 hsv_picker_height = 200.0f;
    real32 hue_slider_width = 20.0f;
    real32 small_padding_x = 6.0f;
    real32 small_padding_y = 6.0f;
    real32 color_picker_width = (small_padding_x +
                                 hsv_picker_width + small_padding_x +
                                 hue_slider_width + small_padding_x);
    real32 color_picker_height = 200.0f + small_padding_y*2;
};

#define MATERIAL_LIBRARY_WINDOW 0x1
#define TEXTURE_LIBRARY_WINDOW  0x2
#define MESH_LIBRARY_WINDOW     0x4

struct Editor_State {
    Transform_Mode transform_mode;

    bool32 use_freecam;
    
    int32 selected_entity_index;
    Entity_Type selected_entity_type;

    int32 last_selected_entity_index;
    Entity_Type last_selected_entity_type;

    Gizmo gizmo;
    Gizmo_Handle hovered_gizmo_handle;
    Gizmo_Handle selected_gizmo_handle;
    Vec3 gizmo_initial_hit;
    Vec3 gizmo_transform_axis;
    Vec3 last_gizmo_transform_point;

    bool32 show_wireframe;
    uint32 open_window_flags;
    Mesh_Type mesh_library_filter;
    bool32 editing_selected_entity_material;
    bool32 editing_selected_entity_mesh;

    bool32 is_new_level;
    String_Buffer current_level_filename;

    Editor_Color_Picker open_color_picker;
    Vec2 color_picker_position;
    void *color_picker_color_pointer;
    UI_id color_picker_parent;
    UI_Color_Picker_State color_picker_state;
};

#endif
