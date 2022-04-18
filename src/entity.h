#ifndef ENTITY_H
#define ENTITY_H

#include "math.h"
#include "string.h"

enum Entity_Type {
    ENTITY_NONE,
    ENTITY_NORMAL,
    ENTITY_POINT_LIGHT,
};

#define ENTITY_HEADER                           \
    Entity_Type type;                           \
    Transform transform;                        \
    int32 mesh_id;                              \
    int32 material_id;

struct Entity {
    ENTITY_HEADER
};

struct Normal_Entity {
    ENTITY_HEADER
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

Material make_material(String_Buffer material_name,
                       int32 texture_id,
                       real32 specular_exponent,
                       Vec4 color_override, bool32 use_color_override) {
    Material material = { material_name, texture_id,
                          specular_exponent, color_override, use_color_override };
    return material;
}

Normal_Entity make_entity(int32 mesh_id,
                          int32 material_id,
                          Transform transform) {
    Normal_Entity entity = { ENTITY_NORMAL, transform,
                             mesh_id, material_id };
    return entity;
}

Texture make_texture(String_Buffer texture_name, String_Buffer filename) {
    Texture texture = {};
    texture.name = texture_name;
    texture.filename = filename;
    return texture;
}

Point_Light_Entity make_point_light_entity(int32 mesh_id,
                                           int32 material_id,
                                           Vec3 light_color,
                                           real32 d_min, real32 d_max,
                                           Transform transform) {
    Point_Light_Entity entity = { ENTITY_POINT_LIGHT, transform,
                                  mesh_id, material_id,
                                  light_color, d_min, d_max };
    return entity;
}

#endif
