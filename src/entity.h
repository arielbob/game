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
    String mesh_name;     // owned by asset manager
    String material_name; // owned by asset manager
    AABB transformed_aabb;
    Collider_Variant collider;
};

void deallocate(Normal_Entity *entity) {
    // nothing to deallocate
}

struct Point_Light_Entity {
    Vec3 light_color;
    real32 falloff_start;
    real32 falloff_end;
};

void deallocate(Point_Light_Entity *entity) {
    // nothing to deallocate
}

struct Entity {
    int32 id;
    Entity_Type type;
    Transform transform;
    union {
        Normal_Entity normal;
        Point_Light_Entity point_light;
    };

    Entity *prev;
    Entity *next;
};

void deallocate(Entity *entity) {
    switch (entity->type) {
    case ENTITY_NORMAL: {
        deallocate(&entity->normal);
    } break;
    case ENTITY_POINT_LIGHT: {
        deallocate(&entity->point_light);
    } break;
    case ENTITY_NONE: {
        assert(!"Entity has no type");
    } break;
    default: {
        assert(!"Unhandled entity type");
    }
    }
}

#endif
