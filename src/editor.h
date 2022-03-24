#ifndef EDITOR_H
#define EDITOR_H

struct Gizmo {
    Transform transform;
    char *arrow_mesh_name;
};

struct Editor_State {
    int32 selected_entity_index;
    Gizmo gizmo;
};

#endif
