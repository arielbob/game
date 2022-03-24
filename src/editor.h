#ifndef EDITOR_H
#define EDITOR_H

enum Transform_Mode {
    TRANSFORM_GLOBAL,
    TRANSFORM_LOCAL
};

struct Gizmo {
    Transform transform;
    char *arrow_mesh_name;
};

struct Editor_State {
    Transform_Mode transform_mode;
    int32 selected_entity_index;
    Gizmo gizmo;
};

#endif
