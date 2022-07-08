#ifndef ENTITY_H
#define ENTITY_H

#include "platform.h"
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

    String mesh_name;
    String material_name;
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

Buffer serialize(Allocator *allocator, Normal_Entity entity) {
    uint32 offset = 0;
    offset += sizeof(Normal_Entity);
    
    uint32 mesh_name_offset = offset;
    offset += entity.mesh_name.length;

    uint32 material_name_offset = offset;
    offset += entity.material_name.length;

    uint8 *data = (uint8 *) allocate(allocator, offset);

    memcpy(&data[mesh_name_offset],     entity.mesh_name.contents,     entity.mesh_name.length);
    memcpy(&data[material_name_offset], entity.material_name.contents, entity.material_name.length);

    entity.mesh_name.contents =     (char *) (data + mesh_name_offset);
    entity.material_name.contents = (char *) (data + material_name_offset);
    *((Normal_Entity *) data) = entity;

    Buffer result;
    result.data = data;
    result.size = offset;
    
    return result;
}

Normal_Entity deserialize_normal_entity(Allocator *allocator, Buffer buffer) {
    Normal_Entity result = *((Normal_Entity *) buffer.data);
    result.mesh_name = copy(allocator, result.mesh_name);
    result.material_name = copy(allocator, result.material_name);
    return result;
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
