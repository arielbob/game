#ifndef ENTITY_2_H
#define ENTITY_2_H

#include "platform.h"
#include "math.h"
#include "string.h"
#include "asset.h"
#include "collider.h"
#include "level.h"

#define ENTITY_MESH           (1 << 0)
#define ENTITY_MATERIAL       (1 << 1)
#define ENTITY_COLLIDER       (1 << 2)
#define ENTITY_LIGHT          (1 << 3) // this is used to say that an entity is a light

// this basically decides which of the light fields to use
// for example, LIGHT_POINT will use falloff_start and falloff_end, and ignore
// all the other light properties. the light_color field is shared across all
// light types.
// NOTE: for now, we just assume that falloff_start and falloff_end are set when
//       light_type is set. this is fine for now, but we may want to have flags
//       for light fields if they need to be deallocated.
enum Light_Type {
    LIGHT_POINT
};

#define ENTITY_FIELDS                           \
    uint32 flags;                               \
    Transform transform;                        \
                                                \
    int32 mesh_id;                              \
    int32 material_id;                          \
    AABB transformed_aabb;                      \
                                                \
    Collider_Variant collider;                  \
                                                \
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
        // nothing to do
    }

    if (entity->flags & ENTITY_MATERIAL) {
        // nothing to do
    }
}

void set_material(Entity *entity, int32 material_id);
void set_mesh(Entity *entity, int32 id);

#endif
