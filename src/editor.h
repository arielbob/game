#ifndef EDITOR_H
#define EDITOR_H

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
    char *arrow_mesh_name;
    char *ring_mesh_name;
    char *sphere_mesh_name;
};

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

    bool32 choosing_material;
    bool32 editing_selected_entity_material;
    
    Material temp_material;
};

#endif
