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
    int32 id;                                   \
    Transform transform;                        \

struct Entity {
    ENTITY_HEADER
};

void deallocate(Entity *) {
    // nothing to deallocate. usually entity structs are deallocated another way and not through this procedure.
}

struct Normal_Entity {
    ENTITY_HEADER

    int32 mesh_id;
    int32 material_id;
    AABB transformed_aabb;
    Collider_Variant collider;
    bool32 is_walkable;
};

Normal_Entity make_normal_entity() {
    Normal_Entity result = {};
    result.transform = make_transform();
    result.type = ENTITY_NORMAL;
    result.mesh_id = -1;
    result.material_id = -1;
    return result;
}

Normal_Entity make_normal_entity(Transform transform, int32 mesh_id, int32 material_id, AABB transformed_aabb) {
    Normal_Entity result = {};
    result.type = ENTITY_NORMAL;
    result.transform = transform;
    result.mesh_id = mesh_id;
    result.material_id = material_id;
    result.transformed_aabb = transformed_aabb;
    return result;
}

void deallocate(Normal_Entity entity) {
    // nothing to deallocate
}

Normal_Entity copy(Allocator *allocator, Normal_Entity entity) {
    return entity;
}

struct Point_Light_Entity {
    ENTITY_HEADER

    Vec3 light_color;
    real32 falloff_start;
    real32 falloff_end;
};

Point_Light_Entity make_point_light_entity() {
    Point_Light_Entity result = {};
    result.type = ENTITY_POINT_LIGHT;
    result.transform = make_transform();
    return result;
}

Point_Light_Entity make_point_light_entity(Vec3 light_color, real32 falloff_start, real32 falloff_end) {
    Point_Light_Entity result = {};
    result.type = ENTITY_POINT_LIGHT;
    result.transform = make_transform();

    result.light_color = light_color;
    result.falloff_start = falloff_start;
    result.falloff_end = falloff_end;

    return result;
}

void deallocate(Point_Light_Entity entity) {
    // nothing to deallocate
}

Point_Light_Entity copy(Allocator *allocator, Point_Light_Entity entity) {
    return entity;
}

#endif
