#ifndef COLLIDER_H
#define COLLIDER_H

enum class Collider_Type {
    NONE, CIRCLE
};

char *get_collider_type_string(Collider_Type type) {
    switch (type) {
        case Collider_Type::NONE:
            return "None";
        case Collider_Type::CIRCLE:
            return "Circle";
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

inline Circle_Collider make_circle_collider(Vec3 center, real32 radius) {
    return { Collider_Type::CIRCLE, center, radius };
}

struct Collider_Variant {
    Collider_Type type;
    union {
        Circle_Collider circle;
    };
};

#endif
