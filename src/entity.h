#ifndef ENTITY_2_H
#define ENTITY_2_H

#include "platform.h"
#include "math.h"
#include "string.h"
#include "asset.h"
#include "collider.h"

#define ENTITY_MESH           (1 << 0)
#define ENTITY_MATERIAL       (1 << 1)
#define ENTITY_COLLIDER       (1 << 2)
#define ENTITY_LIGHT          (1 << 3) // this is used to say that an entity is a light

// this basically decides which of the light fields to use
// for example, LIGHT_POINT will use falloff_start and falloff_end, and ignore
// all the other light properties. the light_color field is shared across all
// light types.
enum Light_Type {
    LIGHT_POINT
};

#define ENTITY_FIELDS                           \
    uint32 flags;                               \
    Transform transform;                        \
                                                \
    String mesh_name;                           \
    String material_name;                       \
    AABB transformed_aabb;                      \
                                                \
    Collider_Variant collider;                  \
    Light_Type light_type;                      \
    Vec3 light_color;                           \
    real32 falloff_start;                       \
    real32 falloff_end;                         \     

struct Entity {
    int32 id;

    ENTITY_FIELDS
    
    Entity *prev;
    Entity *next;
};

void deallocate(Entity *entity) {
    if (entity->flags & ENTITY_MESH) {
        deallocate(entity->mesh_name);
    }

    if (entity->flags & ENTITY_MATERIAL) {
        deallocate(entity->material_name);
    }
}

#endif
