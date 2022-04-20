#ifndef EDITOR_H
#define EDITOR_H

#define LEVEL_FILE_FILTER_TITLE "Levels (*.level)"
#define LEVEL_FILE_FILTER_TYPE "level"


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
#if 0
    char *arrow_mesh_name;
    char *ring_mesh_name;
    char *sphere_mesh_name;
#endif
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
    bool32 editing_selected_entity_material;
    bool32 editing_selected_entity_mesh;
};

#endif
