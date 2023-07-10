#if !defined (GAME_MATH_H)

#undef min
#undef max

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define EPSILON 1e-9f

#define RGB_MIN 0.0f
#define RGB_MAX 255.0f

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
    inline bool32 operator==(Vec2 v);
};

struct Vec2_int32 {
    union {
        int32 values[2];
        struct {
            int32 x;
            int32 y;
        };
    };

    inline Vec2_int32 &operator+=(Vec2_int32 v);
    inline Vec2_int32 &operator-=(Vec2_int32 v);
    inline Vec2_int32 &operator*=(int32 factor);
    inline Vec2_int32 &operator/=(int32 factor);
    inline int32 &operator[](int32 index);
    inline bool32 operator==(Vec2_int32 v);
    inline bool32 operator!=(Vec2_int32 v);
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

Vec3 x_axis = { 1.0f, 0.0f, 0.0f };
Vec3 y_axis = { 0.0f, 1.0f, 0.0f };
Vec3 z_axis = { 0.0f, 0.0f, 1.0f };

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

struct Capsule {
    Vec3 base; // the bottom of the capsule
    Vec3 tip;  // the top of the capsule
    real32 radius;
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

// rotation members are in degrees
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

struct LRTB_Rect_int32 {
    int32 left;
    int32 right;
    int32 top;
    int32 bottom;
};

inline Vec3 make_vec3(real32 x, real32 y, real32 z);
inline Vec4 make_vec4(real32 x, real32 y, real32 z, real32 w);
inline Vec4 make_vec4(Vec3 v, real32 w);
Vec4 rgba_to_vec4(real32 r, real32 g, real32 b, real32 a);
Vec4 rgb_to_vec4(real32 r, real32 g, real32 b);
RGB_Color vec3_to_rgb(Vec3 v);
HSV_Color rgb_to_hsv(RGB_Color rgb_color);
AABB transform_aabb(AABB aabb, Mat4 transform_matrix);
Mat4 get_model_matrix(Transform transform);
real32 clamp(real32 value, real32 min, real32 max);
Quaternion make_quaternion();
Transform make_transform();
Transform make_transform(Vec3 position, Quaternion rotation, Vec3 scale);
inline Vec3 truncate_v4_to_v3(Vec4 vec4);
inline Vec3 operator+(Vec3 v1, Vec3 v2);
Capsule make_capsule(Vec3 base, Vec3 tip, real32 radius);

#define GAME_MATH_H
#endif
