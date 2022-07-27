#ifndef ENTITY_2_H
#define ENTITY_2_H

#include "platform.h"
#include "math.h"
#include "string.h"
#include "asset.h"
#include "collider.h"

enum Entity_Type {
    ENTITY_NONE,
    ENTITY_NORMAL,
    ENTITY_POINT_LIGHT
};

struct Normal_Entity {
    Transform transform;
    char *mesh_name;
    char *material_name;
    AABB tranfsormed_aabb;
};

struct Point_Light_Entity {
    Vec3 light_color;
    real32 falloff_start;
    real32 falloff_end;
}

struct Entity {
    Entity_Type type;
    Transform transform;
    union {
        Normal_Entity normal;
        Point_Light_Entity point_light;
    };
    Entity *next;
};

#endif
