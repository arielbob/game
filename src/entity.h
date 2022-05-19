#ifndef ENTITY_H
#define ENTITY_H

#include "math.h"
#include "string.h"
#include "mesh.h"
#include "collider.h"

enum Entity_Type {
    ENTITY_NONE,
    ENTITY_NORMAL,
    ENTITY_POINT_LIGHT,
    ENTITY_WALK_MESH
};

#define ENTITY_HEADER                           \
    Entity_Type type;                           \
    Transform transform;                        \

struct Entity {
    ENTITY_HEADER
};

struct Normal_Entity {
    ENTITY_HEADER

    int32 mesh_id;
    int32 material_id;
    AABB transformed_aabb;
    Collider_Variant collider;
    bool32 is_walkable;
};

struct Point_Light_Entity {
    ENTITY_HEADER

    Vec3 light_color;
    real32 falloff_start;
    real32 falloff_end;
};

Normal_Entity make_normal_entity() {
    Normal_Entity result = {};
    result.type = ENTITY_NORMAL;
    result.mesh_id = -1;
    result.material_id = -1;
    return result;
}

Point_Light_Entity make_point_light_entity() {
    Point_Light_Entity result = {};
    result.type = ENTITY_POINT_LIGHT;
    return result;
}

#endif
