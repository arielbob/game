#ifndef COLLIDER_H
#define COLLIDER_H

#include "math.h"

enum class Collider_Type {
    NONE, CIRCLE, CAPSULE
};

char *get_collider_type_string(Collider_Type type) {
    switch (type) {
        case Collider_Type::NONE:
            return "None";
        case Collider_Type::CIRCLE:
            return "Circle";
        case Collider_Type::CAPSULE:
            return "Capsule";
        default: {
            assert(!"Unhandled collider type.");
            return "";
        }
    }
}

#define COLLIDER_HEADER                         \
    Collider_Type type;

// circle collider is a circle on the xz-plane
struct Circle_Collider {
    COLLIDER_HEADER
    
    Vec3 center;
    real32 radius;
};

struct Capsule_Collider {
    COLLIDER_HEADER
    
    Capsule capsule;
};

inline Circle_Collider make_circle_collider(Vec3 center, real32 radius) {
    return { Collider_Type::CIRCLE, center, radius };
}

inline Capsule_Collider make_capsule_collider(Vec3 base, Vec3 normal, real32 radius) {
    Vec3 tip = base + normal;
    Capsule capsule = make_capsule(base, tip, radius);
    return { Collider_Type::CAPSULE, capsule };
}

struct Collider_Variant {
    Collider_Type type;
    union {
        Circle_Collider circle;
        Capsule_Collider capsule;
    };
};

#endif
