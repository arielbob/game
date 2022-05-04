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

    Mesh_Type mesh_type;
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

struct Texture {
    String_Buffer name;
    String_Buffer filename;
    bool32 is_loaded;
    bool32 should_unload;
};

struct Material {
    String_Buffer name;
    int32 texture_id;
    real32 gloss;
    Vec4 color_override;
    bool32 use_color_override;
};

Material default_material = {
    make_empty_string_buffer("", 0),
    -1,
    0.5f,
    make_vec4(0.5f, 0.5f, 0.5f, 1.0f),
    true
};

Material make_material(String_Buffer material_name,
                       int32 texture_id,
                       real32 specular_exponent,
                       Vec4 color_override, bool32 use_color_override) {
    Material material = { material_name, texture_id,
                          specular_exponent, color_override, use_color_override };
    return material;
}

Normal_Entity make_entity(Mesh_Type mesh_type, int32 mesh_id,
                          int32 material_id,
                          Transform transform, AABB aabb,
                          Collider_Variant collider,
                          bool32 is_walkable = false) {
    AABB transformed_aabb = transform_aabb(aabb, get_model_matrix(transform));
    Normal_Entity entity = { ENTITY_NORMAL, transform,
                             mesh_type, mesh_id, material_id,
                             transformed_aabb,
                             collider,
                             is_walkable };
    return entity;
}

Normal_Entity make_entity(Mesh_Type mesh_type, int32 mesh_id,
                          int32 material_id,
                          Transform transform, AABB aabb,
                          bool32 is_walkable = false) {
    AABB transformed_aabb = transform_aabb(aabb, get_model_matrix(transform));
    Collider_Variant collider;
    collider.type = Collider_Type::NONE;
    Normal_Entity entity = { ENTITY_NORMAL, transform,
                             mesh_type, mesh_id, material_id,
                             transformed_aabb, collider,
                             is_walkable };
    return entity;
}

Texture make_texture(String_Buffer texture_name, String_Buffer filename) {
    Texture texture = {};
    texture.name = texture_name;
    texture.filename = filename;
    return texture;
}

Point_Light_Entity make_point_light_entity(Vec3 light_color,
                                           real32 d_min, real32 d_max,
                                           Transform transform) {
    Point_Light_Entity entity = { ENTITY_POINT_LIGHT, transform,
                                  light_color, d_min, d_max };
    return entity;
}

void deallocate(Normal_Entity entity) {
    // nothing to deallocate for now
}

void deallocate(Point_Light_Entity entity) {
    // nothing to deallocate for now
}

void deallocate(Texture texture) {
    delete_string_buffer(texture.name);
    delete_string_buffer(texture.filename);
}

void deallocate(Material material) {
    delete_string_buffer(material.name);
}

#endif
