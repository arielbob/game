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
    int32 mesh_index;                           \
    int32 material_index;

struct Entity {
    ENTITY_HEADER
};

struct Normal_Entity {
    ENTITY_HEADER
};

struct Point_Light_Entity {
    ENTITY_HEADER

    Vec3 light_color;
    real32 d_min;
    real32 d_max;
};

struct Material {
    String_Buffer name;
    String_Buffer texture_name;
    real32 gloss;
    Vec4 color_override;
    bool32 use_color_override;
};

/*
  we call do_slider()
  in Slider_State, we allocate a String_Buffer,
  at the end of the game update procedure, we do the same thing we do for removing hot if the element no longer
  exists. except this time, if it no longer exists, we delete the state object. to delete the state object,
  in the hash map in which the UI element states are stored, we make that slot free. and we also delete the
  string buffer.

  this requires:
  - TODO (done): hash map implementation that uses open addressing
  - TODO (done): figure out memory alignment
  - TODO (done): pool allocator for strings (fixed size array with ability to allocate and deallocate)
  - TODO: some type of hash map implementation that can store base classes (without having to store pointers)
    - just use the new hash map implementation with a variant struct, which is just a union of all the derived
      structs
 */

#endif
