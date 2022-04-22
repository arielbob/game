#if !defined (GAME_MATH_H)

#undef min
#undef max

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define EPSILON 1e-9

struct Vec2 {
    union {
        real32 values[2];
        struct {
            real32 x;
            real32 y;
        };
    };

    inline Vec2 &operator+=(Vec2 v);
    inline Vec2 &operator-=(Vec2 v);
    inline Vec2 &operator*=(real32 factor);
    inline Vec2 &operator/=(real32 factor);
    inline real32 &operator[](int32 index);
};

struct Vec3 {
    union {
        real32 values[3];
        struct {
            real32 x;
            real32 y;
            real32 z;
        };
    };

    inline Vec3 &operator+=(Vec3 v);
    inline Vec3 &operator-=(Vec3 v);
    inline Vec3 &operator*=(real32 factor);
    inline Vec3 &operator/=(real32 factor);
    inline real32 &operator[](int32 index);
};

struct Vec4 {
    union {
        real32 values[4];
        struct {
            real32 x;
            real32 y;
            real32 z;
            real32 w;
        };
    };
    
    inline Vec4 &operator+=(Vec4 v);
    inline Vec4 &operator-=(Vec4 v);
    inline Vec4 &operator*=(real32 factor);
    inline Vec4 &operator/=(real32 factor);
    inline real32 &operator[](int32 index);
};

// NOTE: we define it this way so we can send it easily to opengl shaders
struct Mat4 {
    union {
        Vec4 values[4];
        struct {
            Vec4 col1;
            Vec4 col2;
            Vec4 col3;
            Vec4 col4;
        };
    };
};

struct Basis {
    Vec3 forward;
    Vec3 right;
    Vec3 up;
};

struct Plane {
    // d * normal = a point on the plane. normal is not necessarily a unit vector, but for some equations
    // it is required for it to be a unit vector.
    real32 d;
    Vec3 normal;
};

struct AABB {
    Vec3 p_min;
    Vec3 p_max;
};

struct Ray {
    Vec3 origin;
    Vec3 direction;
};

struct Ray_2D {
    Vec2 origin;
    Vec2 direction;
};

struct Line {
    Vec3 origin;
    Vec3 line; // NOT necessarily a unit vector, since a line is of a certain length
};

struct Cylinder {
    real32 radius;
    real32 phi_max_degrees;
    real32 height;
};

struct Quaternion {
    real32 w;
    Vec3 v;
};

struct Transform {
    Vec3 position;
    Quaternion rotation;
    Vec3 scale;
};

struct Euler_Transform {
    Vec3 position;
    real32 heading;
    real32 pitch;
    real32 roll;
    Vec3 scale;
};

// TODO: we may want to just have these be uint8's?
struct RGB_Color {
    real32 r;
    real32 g;
    real32 b;
};

struct HSV_Color {
    real32 h;
    real32 s;
    real32 v;
};

inline Vec4 make_vec4(real32 x, real32 y, real32 z, real32 w);
Vec4 rgba_to_vec4(real32 r, real32 g, real32 b, real32 a);
Vec4 rgb_to_vec4(real32 r, real32 g, real32 b);
//Vec4 rgb_to_vec4(int32 r, int32 g, int32 b);

#define GAME_MATH_H
#endif
