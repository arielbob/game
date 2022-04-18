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
};

struct Material {
    String_Buffer name;
    int32 texture_id;
    real32 gloss;
    Vec4 color_override;
    bool32 use_color_override;
};

#endif
