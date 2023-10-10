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
    LIGHT_POINT,
    LIGHT_SUN
};

// TODO: rename the falloff_start and _end variables to prepend point_
// - just because we will probably have other light sources that have those same
//   variables
// we keep them all instead of using a union (like we do with shader uniforms in
// UI_Theme) because it makes parsing easier. we don't have to any checking or
// making sure an order is kept; we can just read the parameter and set it. it
// also gives us the ability to have parameters for light types other than whatever
// light_type is set to. so you can switch either in the editor or in the game and
// all the params will be saved. all this at the expense of a larger struct,
// obviously, but i think it's fine.
// all this being said, currently, in level_export.cpp, we actually only export
// the parameters for the current light_type. yeah, it's kinda weird and
// asymmetrical... idk. it's honestly useful for editing and for parsing.
// let's just keep it this way. i guess when we save, it's expected the unused
// params will be dropped. i guess we just don't support switching light types
// yet in-game.

#define POINT_LIGHT_FIELDS                      \
    Vec3 light_color;                           \
    real32 point_light_intensity;               \
    real32 falloff_start;                       \
    real32 falloff_end;                         \

// note that we don't store another Vec3 for the direction; we just use
// the rotation transform of the entity
#define SUN_LIGHT_FIELDS                        \
    Vec3 sun_color;                             \

#define ENTITY_FIELDS                           \
    uint32 flags;                               \
    Transform transform;                        \
                                                \
    int32 mesh_id = ENGINE_DEFAULT_CUBE_MESH_ID;\
    int32 material_id;                          \
    AABB transformed_aabb;                      \
                                                \
    Collider_Variant collider;                  \
                                                \
    Light_Type light_type;                      \
    POINT_LIGHT_FIELDS                          \
    SUN_LIGHT_FIELDS                            \

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
