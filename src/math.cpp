#include "math.h"

Ray make_ray(Vec3 origin, Vec3 direction) {
    Ray result;
    result.origin = origin;
    result.direction = direction;
    return result;
}

Line make_line(Vec3 origin, Vec3 line) {
    Line result;
    result.origin = origin;
    result.line = line;
    return result;
}

Capsule make_capsule(Vec3 base, Vec3 tip, real32 radius) {
    return { base, tip, radius };
}

inline Vec2 make_vec2(real32 x, real32 y) {
    Vec2 vec2;
    vec2.x = x;
    vec2.y = y;

    return vec2;
};

inline Vec2 make_vec2(Vec3 v) {
    Vec2 vec2;
    vec2.x = v.x;
    vec2.y = v.y;

    return vec2;
};

inline Vec2 operator+(Vec2 v1, Vec2 v2) {
    Vec2 result = v1;
    result.x += v2.x;
    result.y += v2.y;
    return result;
}

inline Vec2 operator-(Vec2 v) {
    Vec2 result;
    result.x = -v.x;
    result.y = -v.y;
    return result;
}

inline Vec2 operator-(Vec2 v1, Vec2 v2) {
    Vec2 result = v1;
    result.x -= v2.x;
    result.y -= v2.y;
    return result;
}

inline Vec2 operator*(real32 factor, Vec2 v) {
    Vec2 result = v;
    result.x *= factor;
    result.y *= factor;
    return result;
}

inline Vec2 operator*(Vec2 v, real32 factor) {
    return factor * v;
}

inline Vec2 operator/(Vec2 v, real32 factor) {
    Vec2 result = v;
    result.x /= factor;
    result.y /= factor;
    return result;
}

inline Vec2& Vec2::operator+=(Vec2 v) {
    *this = *this + v;
    return *this;
}

inline Vec2& Vec2::operator-=(Vec2 v) {
    *this = *this - v;
    return *this;
}

inline Vec2& Vec2::operator*=(real32 factor) {
    *this = *this * factor;
    return *this;
}

inline Vec2& Vec2::operator/=(real32 factor) {
    *this = *this / factor;
    return *this;
}

inline real32& Vec2::operator[](int32 index) {
    return (*this).values[index];
}

inline bool32 Vec2::operator==(Vec2 v) {
    return (x == v.x && y == v.y);
}

inline Vec2_int32 make_vec2(int32 x, int32 y) {
    Vec2_int32 vec2;
    vec2.x = x;
    vec2.y = y;

    return vec2;
};

inline Vec2_int32 make_vec2_int32(Vec2 v) {
    return { (int32) v.x, (int32) v.y };
}

inline Vec2_int32 operator+(Vec2_int32 v1, Vec2_int32 v2) {
    Vec2_int32 result = v1;
    result.x += v2.x;
    result.y += v2.y;
    return result;
}

inline Vec2_int32 operator-(Vec2_int32 v) {
    Vec2_int32 result;
    result.x = -v.x;
    result.y = -v.y;
    return result;
}

inline Vec2_int32 operator-(Vec2_int32 v1, Vec2_int32 v2) {
    Vec2_int32 result = v1;
    result.x -= v2.x;
    result.y -= v2.y;
    return result;
}

inline Vec2_int32 operator*(int32 factor, Vec2_int32 v) {
    Vec2_int32 result = v;
    result.x *= factor;
    result.y *= factor;
    return result;
}

inline Vec2_int32 operator*(Vec2_int32 v, int32 factor) {
    return factor * v;
}

inline Vec2_int32 operator/(Vec2_int32 v, int32 factor) {
    Vec2_int32 result = v;
    result.x /= factor;
    result.y /= factor;
    return result;
}

inline Vec2_int32& Vec2_int32::operator+=(Vec2_int32 v) {
    *this = *this + v;
    return *this;
}

inline Vec2_int32& Vec2_int32::operator-=(Vec2_int32 v) {
    *this = *this - v;
    return *this;
}

inline Vec2_int32& Vec2_int32::operator*=(int32 factor) {
    *this = *this * factor;
    return *this;
}

inline Vec2_int32& Vec2_int32::operator/=(int32 factor) {
    *this = *this / factor;
    return *this;
}

inline int32& Vec2_int32::operator[](int32 index) {
    return (*this).values[index];
}

inline bool32 Vec2_int32::operator==(Vec2_int32 v) {
    return (x == v.x && y == v.y);
}

inline bool32 Vec2_int32::operator!=(Vec2_int32 v) {
    return (x != v.x || y != v.y);
}

inline Vec3 operator+(Vec3 v1, Vec3 v2) {
    Vec3 result = v1;
    result.x += v2.x;
    result.y += v2.y;
    result.z += v2.z;
    return result;
}

inline Vec3 operator-(Vec3 v) {
    Vec3 result;
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    return result;
}

inline Vec3 operator-(Vec3 v1, Vec3 v2) {
    Vec3 result = v1;
    result.x -= v2.x;
    result.y -= v2.y;
    result.z -= v2.z;
    return result;
}

inline Vec3 operator*(real32 factor, Vec3 v) {
    Vec3 result = v;
    result.x *= factor;
    result.y *= factor;
    result.z *= factor;
    return result;
}

inline Vec3 operator*(Vec3 v, real32 factor) {
    return factor * v;
}

inline Vec3 operator/(Vec3 v, real32 factor) {
    Vec3 result = v;
    result.x /= factor;
    result.y /= factor;
    result.z /= factor;
    return result;
}

inline Vec3& Vec3::operator+=(Vec3 v) {
    *this = *this + v;
    return *this;
}

inline Vec3& Vec3::operator-=(Vec3 v) {
    *this = *this - v;
    return *this;
}

inline Vec3& Vec3::operator*=(real32 factor) {
    *this = *this * factor;
    return *this;
}

inline Vec3& Vec3::operator/=(real32 factor) {
    *this = *this / factor;
    return *this;
}

inline real32& Vec3::operator[](int32 index) {
    return (*this).values[index];
}

inline Vec3 make_vec3() {
    Vec3 vec3 = {};
    return vec3;
};

inline Vec3 make_vec3(Vec2 v, real32 z) {
    Vec3 vec3 = {};
    vec3.x = v.x;
    vec3.y = v.y;
    vec3.z = z;
    return vec3;
};

inline Vec3 make_vec3(real32 x, real32 y, real32 z) {
    Vec3 vec3;
    vec3.x = x;
    vec3.y = y;
    vec3.z = z;
    return vec3;
};

inline Vec4 operator+(Vec4 v1, Vec4 v2) {
    Vec4 result = v1;
    result.x += v2.x;
    result.y += v2.y;
    result.z += v2.z;
    result.w += v2.w;
    return result;
}

inline Vec4 operator-(Vec4 v) {
    Vec4 result;
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    result.w = -v.w;
    return result;
}

inline Vec4 operator-(Vec4 v1, Vec4 v2) {
    Vec4 result = v1;
    result.x -= v2.x;
    result.y -= v2.y;
    result.z -= v2.z;
    result.w -= v2.w;
    return result;
}

inline Vec4 operator*(real32 factor, Vec4 v) {
    Vec4 result = v;
    result.x *= factor;
    result.y *= factor;
    result.z *= factor;
    result.w *= factor;
    return result;
}

inline Vec4 operator*(Vec4 v, real32 factor) {
    return factor * v;
}

inline Vec4 operator/(Vec4 v, real32 factor) {
    Vec4 result = v;
    result.x /= factor;
    result.y /= factor;
    result.z /= factor;
    result.w /= factor;
    return result;
}

inline Vec4& Vec4::operator+=(Vec4 v) {
    *this = *this + v;
    return *this;
}

inline Vec4& Vec4::operator-=(Vec4 v) {
    *this = *this - v;
    return *this;
}

inline Vec4& Vec4::operator*=(real32 factor) {
    *this = *this * factor;
    return *this;
}

inline Vec4& Vec4::operator/=(real32 factor) {
    *this = *this / factor;
    return *this;
}

inline real32& Vec4::operator[](int32 index) {
    return (*this).values[index];
}

inline Vec4 make_vec4(real32 x, real32 y, real32 z, real32 w) {
    Vec4 vec4;
    vec4.x = x;
    vec4.y = y;
    vec4.z = z;
    vec4.w = w;

    return vec4;
}

inline Vec4 make_vec4(Vec3 v, real32 w) {
    Vec4 result;
    
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;
    result.w = w;
    
    return result;
}

inline Vec3 homogeneous_divide(Vec4 vec4) {
    Vec3 vec3 = {};
    // NOTE: not sure if we need to check for divide by zero here..
    vec3.x = vec4.x / vec4.w;
    vec3.y = vec4.y / vec4.w;
    vec3.z = vec4.z / vec4.w;

    return vec3;
}

Mat4 make_mat4(real32 x11, real32 x12, real32 x13, real32 x14,
               real32 x21, real32 x22, real32 x23, real32 x24,
               real32 x31, real32 x32, real32 x33, real32 x34,
               real32 x41, real32 x42, real32 x43, real32 x44) {
    Mat4 result = {};

    // TODO: square bracket operator function
    // TODO: check if this'll cause a cache miss?
    result.col1.x = x11;
    result.col2.x = x12;
    result.col3.x = x13;
    result.col4.x = x14;

    result.col1.y = x21;
    result.col2.y = x22;
    result.col3.y = x23;
    result.col4.y = x24;

    result.col1.z = x31;
    result.col2.z = x32;
    result.col3.z = x33;
    result.col4.z = x34;

    result.col1.w = x41;
    result.col2.w = x42;
    result.col3.w = x43;
    result.col4.w = x44;
    
    return result;
}

Mat4 make_mat4_identity() {
    Mat4 result = {};
    
    result.col1.x = 1.0f;
    result.col2.y = 1.0f;
    result.col3.z = 1.0f;
    result.col4.w = 1.0f;
    
    return result;
}

inline Mat4 operator*(Mat4 m1, Mat4 m2) {
    Mat4 result = {};

    result.col1.x = m1.col1.x*m2.col1.x + m1.col2.x*m2.col1.y + m1.col3.x*m2.col1.z + m1.col4.x*m2.col1.w;
    result.col2.x = m1.col1.x*m2.col2.x + m1.col2.x*m2.col2.y + m1.col3.x*m2.col2.z + m1.col4.x*m2.col2.w;
    result.col3.x = m1.col1.x*m2.col3.x + m1.col2.x*m2.col3.y + m1.col3.x*m2.col3.z + m1.col4.x*m2.col3.w;
    result.col4.x = m1.col1.x*m2.col4.x + m1.col2.x*m2.col4.y + m1.col3.x*m2.col4.z + m1.col4.x*m2.col4.w;

    result.col1.y = m1.col1.y*m2.col1.x + m1.col2.y*m2.col1.y + m1.col3.y*m2.col1.z + m1.col4.y*m2.col1.w;
    result.col2.y = m1.col1.y*m2.col2.x + m1.col2.y*m2.col2.y + m1.col3.y*m2.col2.z + m1.col4.y*m2.col2.w;
    result.col3.y = m1.col1.y*m2.col3.x + m1.col2.y*m2.col3.y + m1.col3.y*m2.col3.z + m1.col4.y*m2.col3.w;
    result.col4.y = m1.col1.y*m2.col4.x + m1.col2.y*m2.col4.y + m1.col3.y*m2.col4.z + m1.col4.y*m2.col4.w;
    
    result.col1.z = m1.col1.z*m2.col1.x + m1.col2.z*m2.col1.y + m1.col3.z*m2.col1.z + m1.col4.z*m2.col1.w;
    result.col2.z = m1.col1.z*m2.col2.x + m1.col2.z*m2.col2.y + m1.col3.z*m2.col2.z + m1.col4.z*m2.col2.w;
    result.col3.z = m1.col1.z*m2.col3.x + m1.col2.z*m2.col3.y + m1.col3.z*m2.col3.z + m1.col4.z*m2.col3.w;
    result.col4.z = m1.col1.z*m2.col4.x + m1.col2.z*m2.col4.y + m1.col3.z*m2.col4.z + m1.col4.z*m2.col4.w;

    result.col1.w = m1.col1.w*m2.col1.x + m1.col2.w*m2.col1.y + m1.col3.w*m2.col1.z + m1.col4.w*m2.col1.w;
    result.col2.w = m1.col1.w*m2.col2.x + m1.col2.w*m2.col2.y + m1.col3.w*m2.col2.z + m1.col4.w*m2.col2.w;
    result.col3.w = m1.col1.w*m2.col3.x + m1.col2.w*m2.col3.y + m1.col3.w*m2.col3.z + m1.col4.w*m2.col3.w;
    result.col4.w = m1.col1.w*m2.col4.x + m1.col2.w*m2.col4.y + m1.col3.w*m2.col4.z + m1.col4.w*m2.col4.w;
    
    return result;
}

inline Mat4 operator/(Mat4 m, real32 factor) {
    Mat4 result = m;

    result.col1 /= factor;
    result.col2 /= factor;
    result.col3 /= factor;
    result.col4 /= factor;

    return result;
}

inline real32 determinant(real32 m11, real32 m12,
                          real32 m21, real32 m22) {
    return (m11 * m22) - (m12 * m21);
}

real32 determinant(real32 m11, real32 m12, real32 m13,
                   real32 m21, real32 m22, real32 m23,
                   real32 m31, real32 m32, real32 m33) {
    real32 a = m11 * determinant(m22, m23,
                                 m32, m33);
    real32 b = -m12 * determinant(m21, m23,
                                  m31, m33);
    real32 c = m13 * determinant(m21, m22,
                                 m31, m32);
    return a + b + c;
}

Mat4 transpose(Mat4 m) {
    return make_mat4(m.col1.x, m.col1.y, m.col1.z, m.col1.w,
                     m.col2.x, m.col2.y, m.col2.z, m.col2.w,
                     m.col3.x, m.col3.y, m.col3.z, m.col3.w,
                     m.col4.x, m.col4.y, m.col4.z, m.col4.w);
}

Mat4 classical_adjoint(Mat4 m) {
    real32 m11 = m.col1.x;
    real32 m12 = m.col2.x;
    real32 m13 = m.col3.x;
    real32 m14 = m.col4.x;

    real32 m21 = m.col1.y;
    real32 m22 = m.col2.y;
    real32 m23 = m.col3.y;
    real32 m24 = m.col4.y;

    real32 m31 = m.col1.z;
    real32 m32 = m.col2.z;
    real32 m33 = m.col3.z;
    real32 m34 = m.col4.z;

    real32 m41 = m.col1.w;
    real32 m42 = m.col2.w;
    real32 m43 = m.col3.w;
    real32 m44 = m.col4.w;

    real32 c11 = determinant(m22, m23, m24,
                             m32, m33, m34,
                             m42, m43, m44);
    real32 c12 = -determinant(m21, m23, m24,
                              m31, m33, m34,
                              m41, m43, m44);
    real32 c13 = determinant(m21, m22, m24,
                             m31, m32, m34,
                             m41, m42, m44);
    real32 c14 = -determinant(m21, m22, m23,
                              m31, m32, m33,
                              m41, m42, m43);

    real32 c21 = -determinant(m12, m13, m14,
                              m32, m33, m34,
                              m42, m43, m44);
    real32 c22 = determinant(m11, m13, m14,
                             m31, m33, m34,
                             m41, m43, m44);
    real32 c23 = -determinant(m11, m12, m14,
                              m31, m32, m34,
                              m41, m42, m44);
    real32 c24 = determinant(m11, m12, m13,
                             m31, m32, m33,
                             m41, m42, m43);

    real32 c31 = determinant(m12, m13, m14,
                             m22, m23, m24,
                             m42, m43, m44);
    real32 c32 = -determinant(m11, m13, m14,
                              m21, m23, m24,
                              m41, m43, m44);
    real32 c33 = determinant(m11, m12, m14,
                             m21, m22, m24,
                             m41, m42, m44);
    real32 c34 = -determinant(m11, m12, m13,
                              m21, m22, m23,
                              m41, m42, m43);

    real32 c41 = -determinant(m12, m13, m14,
                              m22, m23, m24,
                              m32, m33, m34);
    real32 c42 = determinant(m11, m13, m14,
                             m21, m23, m24,
                             m31, m33, m34);
    real32 c43 = -determinant(m11, m12, m14,
                              m21, m22, m24,
                              m31, m32, m34);
    real32 c44 = determinant(m11, m12, m13,
                             m21, m22, m23,
                             m31, m32, m33);

    // create the transpose of the matrix of cofactors
    return make_mat4(c11, c21, c31, c41,
                     c12, c22, c32, c42,
                     c13, c23, c33, c43,
                     c14, c24, c34, c44);
}

real32 determinant(Mat4 m) {
    real32 m11 = m.col1.x;
    real32 m12 = m.col2.x;
    real32 m13 = m.col3.x;
    real32 m14 = m.col4.x;

    real32 m21 = m.col1.y;
    real32 m22 = m.col2.y;
    real32 m23 = m.col3.y;
    real32 m24 = m.col4.y;

    real32 m31 = m.col1.z;
    real32 m32 = m.col2.z;
    real32 m33 = m.col3.z;
    real32 m34 = m.col4.z;

    real32 m41 = m.col1.w;
    real32 m42 = m.col2.w;
    real32 m43 = m.col3.w;
    real32 m44 = m.col4.w;

    real32 result = (m11 * (m22 * (m33*m44 - m34*m43) + m23 * (m34*m42 - m32*m44) + m24 * (m32*m43 - m33*m42)) -
                     m12 * (m21 * (m33*m44 - m34*m43) + m23 * (m34*m41 - m31*m44) + m24 * (m31*m43 - m33*m41)) +
                     m13 * (m21 * (m32*m44 - m34*m42) + m22 * (m34*m41 - m31*m44) + m24 * (m31*m42 - m32*m41)) -
                     m14 * (m21 * (m32*m43 - m33*m42) + m22 * (m33*m41 - m31*m43) + m23 * (m31*m42 - m32*m41)));
    
    return result;
}

// NOTE: this assumes the matrix to invert is invertible (i.e. non-singular)
Mat4 inverse(Mat4 m) {
    Mat4 adj = classical_adjoint(m);
    Mat4 result = adj / determinant(m);
    return result;
}

// TODO: SIMD
inline Vec4 operator*(Mat4 m, Vec4 v) {
    Vec4 result = {};
    
    real32 sum = 0;
    sum += m.col1.x * v.x;
    sum += m.col2.x * v.y;
    sum += m.col3.x * v.z;
    sum += m.col4.x * v.w;
    result.x = sum;
    
    sum = 0;
    sum += m.col1.y * v.x;
    sum += m.col2.y * v.y;
    sum += m.col3.y * v.z;
    sum += m.col4.y * v.w;
    result.y = sum;
    
    sum = 0;
    sum += m.col1.z * v.x;
    sum += m.col2.z * v.y;
    sum += m.col3.z * v.z;
    sum += m.col4.z * v.w;
    result.z = sum;
    
    sum = 0;
    sum += m.col1.w * v.x;
    sum += m.col2.w * v.y;
    sum += m.col3.w * v.z;
    sum += m.col4.w * v.w;
    result.w = sum;

    return result;
}

inline real32 degs_to_rads(real32 degrees) {
    return degrees * (PI / 180);
}

inline real32 rads_to_degs(real32 radians) {
    return radians * (180 / PI);
}

inline real32 distance(Vec3 v) {
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}

inline Vec3 normalize(Vec3 v) {
    // TODO: i'm not sure why we don't just assert here..
    //       actually i think there is a reason.. back when we were projecting the points on the CPU.
    //       we may want to add the assert back in.
    real32 length = distance(v);
    // TODO: use epsilon value?
    if (length > 0.0f) {
        return v / length;
    } else {
        return v;
    }
}

inline real32 distance(Vec2 v) {
    return sqrtf(v.x*v.x + v.y*v.y);
}

inline Vec2 normalize(Vec2 v) {
    real32 length = distance(v);
    // TODO: use epsilon value?
    // TODO: we may be able to optimize this using max()
    if (length > 0.0f) {
        return v / length;
    } else {
        return v;
    }
}

// TODO: add SIMD version
inline real32 dot(Vec2 v1, Vec2 v2) {
    return v1.x*v2.x + v1.y*v2.y;
}

// TODO: add SIMD version
inline real32 dot(Vec3 v1, Vec3 v2) {
    return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

// TODO: add SIMD version
inline Vec3 cross(Vec3 v1, Vec3 v2) {
    Vec3 result = {};
    result.x = v1.y*v2.z - v1.z*v2.y;
    result.y = v1.z*v2.x - v1.x*v2.z;
    result.z = v1.x*v2.y - v1.y*v2.x;
    return result;
}

inline Quaternion operator*(Quaternion q1, Quaternion q2) {
    Quaternion result;
    
    result.w = q1.w*q2.w - dot(q1.v, q2.v);
    result.v = q1.w*q2.v + q2.w*q1.v + cross(q1.v, q2.v);
    
    return result;
}

// returns unit quaternion
Quaternion make_quaternion() {
    Quaternion result = { 1.0f, 0.0f, 0.0f, 0.0f };
    return result;
}

// NOTE: expects axis to be a unit vector
Quaternion make_quaternion(real32 angle_degs, Vec3 axis) {
    Quaternion result;
    real32 angle_rads = degs_to_rads(angle_degs);
    result.w = cosf(angle_rads / 2.0f);
    result.v = sinf(angle_rads / 2.0f)*axis;
    return result;
}

Quaternion make_quaternion(real32 roll_degs, real32 pitch_degs, real32 heading_degs) {
    Quaternion rotation = make_quaternion(roll_degs, z_axis);
    rotation = make_quaternion(pitch_degs, x_axis) * rotation;
    rotation = make_quaternion(heading_degs, y_axis) * rotation;

    return rotation;
}

inline real32 magnitude(Quaternion q) {
    return sqrtf(q.w*q.w + dot(q.v, q.v));
}

inline Quaternion normalize(Quaternion q) {
    Quaternion result;
    real32 q_magnitude = magnitude(q);
    result.w = q.w / q_magnitude;
    result.v = q.v / q_magnitude;
    return result;
}

Transform make_transform() {
    Transform transform;
    transform.position = {};
    transform.rotation = make_quaternion();
    transform.scale = make_vec3(1.0f, 1.0f, 1.0f);
    return transform;
}

inline Transform make_transform(Vec3 position, Quaternion rotation, Vec3 scale) {
    Transform transform = { position, rotation, scale };
    return transform;
}

inline Quaternion inverse(Quaternion q) {
    // we assume we're using a rotation quaternion (i.e. q.v is a unit vector, and thus magnitude(q) = 1)
    assert(fabsf(magnitude(q) - 1.0f) < EPSILON);
    Quaternion result = { q.w, -q.v };
    return result;
}

inline real32 dot(Quaternion q1, Quaternion q2) {
    return q1.w*q2.w * dot(q1.v, q2.v);
}

Mat4 make_rotate_matrix(Quaternion q) {
    real32 w = q.w;
    real32 x = q.v.x;
    real32 y = q.v.y;
    real32 z = q.v.z;

    real32 x_squared = x*x;
    real32 y_squared = y*y;
    real32 z_squared = z*z;

    Mat4 result = make_mat4(1.0f - 2.0f*y_squared - 2*z_squared, 2.0f*x*y - 2.0f*w*z, 2.0f*x*z + 2.0f*w*y, 0.0f,
                            2.0f*x*y + 2.0f*w*z, 1.0f - 2.0f*x_squared - 2.0f*z_squared, 2.0f*y*z - 2.0f*w*x, 0.0f,
                            2.0f*x*z - 2.0f*w*y, 2.0f*y*z + 2.0f*w*x, 1.0f - 2.0f*x_squared - 2.0f*y_squared, 0.0f,
                            0.0f, 0.0f, 0.0f, 1.0f);

    return result;
}

#if 0
inline Quaternion log(Quaternion q) {
    Quaternion result = { 0.0f, 
}
#endif
    
// TODO: we may just want to return a Mat3, and then
//       overload the multiply function to add in a zero?
//       idk.. that seems slower, but it makes it so we don't store
//       as much space
// TODO: maybe overload this to take in an origin argument?
//       would basically just translate it first, then scale, then translate it back
Mat4 make_scale_matrix(Vec3 axis, real32 factor) {
    Mat4 result = {};
    
    Vec4 col1 = {
        1 + ((factor - 1)*(axis.x*axis.x)),
        (factor - 1)*axis.x*axis.y,
        (factor - 1)*axis.x*axis.z,
        0.0f
    };
    result.col1 = col1;

    Vec4 col2 = {
        (factor - 1)*axis.x*axis.y,        
        1 + ((factor - 1)*(axis.y*axis.y)),
        (factor - 1)*axis.y*axis.z,
        0.0f
    };
    result.col2 = col2;

    Vec4 col3 = {
        (factor - 1)*axis.x*axis.z,
        (factor - 1)*axis.y*axis.z,
        1 + ((factor - 1)*(axis.z*axis.z)),
        0.0f
    };
    result.col3 = col3;
    
    result.col4.w = 1.0f;
    
    return result;
}

Mat4 make_scale_matrix(Vec3 scale) {
    Mat4 result = {};
    
    result.col1.x = scale.x;
    result.col2.y = scale.y;
    result.col3.z = scale.z;
    result.col4.w = 1.0f;
    
    return result;
}

Mat4 make_rotate_matrix(real32 roll, real32 pitch, real32 heading) {
    real32 roll_rads = degs_to_rads(roll);
    real32 pitch_rads = degs_to_rads(pitch);
    real32 heading_rads = degs_to_rads(heading);
    
    Mat4 result = {};

    Vec4 col1 = {
        cosf(heading_rads)*cosf(roll_rads) + sinf(heading_rads)*sinf(pitch_rads)*sinf(roll_rads),
        sinf(roll_rads)*cosf(pitch_rads),
        -sinf(heading_rads)*cosf(roll_rads) + cosf(heading_rads)*sinf(pitch_rads)*sinf(roll_rads),
        0.0f
    };
    result.col1 = col1;

    Vec4 col2 = {
        -cosf(heading_rads)*sinf(roll_rads) + sinf(heading_rads)*sinf(pitch_rads)*cosf(roll_rads),
        cosf(roll_rads)*cosf(pitch_rads),
        sinf(roll_rads)*sinf(heading_rads) + cosf(heading_rads)*sinf(pitch_rads)*cosf(roll_rads),
        0.0f
    };
    result.col2 = col2;

    Vec4 col3 = {
        sinf(heading_rads)*cosf(pitch_rads),
        -sinf(pitch_rads),
        cosf(heading_rads)*cosf(pitch_rads),
        0.0f
    };
    result.col3 = col3;

    result.col4.w = 1.0f;
    
    return result;
}

Mat4 make_rotate_matrix(Vec3 axis, real32 degrees) {
    real32 rads = degs_to_rads(degrees);
    Mat4 result = {};

    // TODO: make sure that cosf(rads) and sinf(rads) are hoisted out when optimized..
    Vec4 col1 = {
        axis.x*axis.x*(1-cosf(rads)) + cosf(rads),
        axis.x*axis.y*(1-cosf(rads)) + axis.z*sinf(rads),
        axis.x*axis.z*(1-cosf(rads)) - axis.y*sinf(rads),
        0.0f
    };
    result.col1 = col1;

    Vec4 col2 = {
        axis.x*axis.y*(1-cosf(rads)) - axis.z*sinf(rads),
        axis.y*axis.y*(1-cosf(rads)) + cosf(rads),
        axis.y*axis.z*(1-cosf(rads)) + axis.x*sinf(rads),
        0.0f
    };
    result.col2 = col2;

    Vec4 col3 = {
        axis.x*axis.z*(1-cosf(rads)) + axis.y*sinf(rads),
        axis.y*axis.z*(1-cosf(rads)) - axis.x*sinf(rads),
        axis.z*axis.z*(1-cosf(rads)) + cosf(rads),
        0.0f
    };
    result.col3 = col3;
    
    result.col4.w = 1.0f;

    return result;
}

Mat4 make_translate_matrix(Vec3 axis, real32 distance) {
    Mat4 result = make_mat4_identity();
    result.col4.x = distance*axis.x;
    result.col4.y = distance*axis.y;
    result.col4.z = distance*axis.z;
    return result;
}

Mat4 make_translate_matrix(Vec3 offset) {
    Mat4 result = make_mat4_identity();
    result.col4.x = offset.x;
    result.col4.y = offset.y;
    result.col4.z = offset.z;
    return result;
}

Mat4 get_view_matrix(Vec3 eye_pos, Vec3 forward, Vec3 right, Vec3 up) {
    // NOTE: we assume that forward, right, and up form an orthonormal basis
    Mat4 result = {};

    result.col1.x = right.x;
    result.col2.x = right.y;
    result.col3.x = right.z;
    result.col4.x = -right.x*eye_pos.x - right.y*eye_pos.y - right.z*eye_pos.z;

    result.col1.y = up.x;
    result.col2.y = up.y;
    result.col3.y = up.z;
    result.col4.y = -up.x*eye_pos.x - up.y*eye_pos.y - up.z*eye_pos.z;
    
    result.col1.z = forward.x;                                                   
    result.col2.z = forward.y;                                                   
    result.col3.z = forward.z;                                                   
    result.col4.z = -forward.x*eye_pos.x - forward.y*eye_pos.y - forward.z*eye_pos.z;

    result.col4.w = 1.0f;
    
    return result;
}

Mat4 make_perspective_clip_matrix(real32 fov_x_degrees, real32 aspect_ratio, real32 near, real32 far) {
    real32 fov_x_rads = degs_to_rads(fov_x_degrees);
    real32 right = tanf(fov_x_rads / 2) * near;
    real32 left = -right;

    real32 top = right / aspect_ratio;
    real32 bottom = -top;

    Mat4 perspective_clip_matrix = {};

    perspective_clip_matrix.col1.x = (2 * near) / (right - left);
    perspective_clip_matrix.col3.x = (-right - left) / (right - left);
    perspective_clip_matrix.col2.y = (2.0f * near) / (top - bottom);
    perspective_clip_matrix.col3.y = (-top - bottom) / (top - bottom);
    perspective_clip_matrix.col3.z = (near + far) / (far - near);
    perspective_clip_matrix.col4.z = (-2 * near * far) / (far - near);
    perspective_clip_matrix.col3.w = 1.0f;

    return perspective_clip_matrix;
}

Mat4 make_ortho_clip_matrix(real32 width, real32 height, real32 near, real32 far) {
    Mat4 m = make_mat4_identity();
    m.col1.x = 2.0f / width;
    m.col4.x = -1.0f;
    m.col2.y = -2.0f / height;
    m.col4.y = 1.0f;
    m.col3.z = 2.0f / (far - near);
    m.col4.z = ((-2.0f * near) / (far - near)) - 1.0f;

    return m;
}

inline Vec3 truncate_v4_to_v3(Vec4 vec4) {
    Vec3 result = { vec4.x, vec4.y, vec4.z };
    return result;
}

// returns false if triangle is degenerate, true otherwise
bool32 compute_barycentric_coords(Vec3 p1, Vec3 p2, Vec3 p3,
                                  Vec3 triangle_normal, Vec3 point,
                                  Vec3 *result) {

    real32 p_x, p_y, x_1, x_2, x_3, y_1, y_2, y_3;
    if (fabs(triangle_normal.x) > fabs(triangle_normal.y) &&
        fabs(triangle_normal.x) > fabs(triangle_normal.z)) {
        // drop out x coord
        p_x = point.y;
        p_y = point.z;
        x_1 = p1.y;
        x_2 = p2.y;
        x_3 = p3.y;
        y_1 = p1.z;
        y_2 = p2.z;
        y_3 = p3.z;
    } else if (fabs(triangle_normal.y) > fabs(triangle_normal.z)) {
        // drop out y coord
        p_x = point.x;
        p_y = point.z;
        x_1 = p1.x;
        x_2 = p2.x;
        x_3 = p3.x;
        y_1 = p1.z;
        y_2 = p2.z;
        y_3 = p3.z;
    } else {
        // drop out z coord
        p_x = point.x;
        p_y = point.y;
        x_1 = p1.x;
        x_2 = p2.x;
        x_3 = p3.x;
        y_1 = p1.y;
        y_2 = p2.y;
        y_3 = p3.y;
    }
    
    real32 denom = (y_1-y_3)*(x_2-x_3) + (y_2-y_3)*(x_3-x_1);

    // check for triangle of zero area
    if (fabs(denom) >= EPSILON) {
        real32 one_over_denom = 1.0f / denom;

        real32 b1, b2, b3;
        b1 = ((p_y-y_3)*(x_2-x_3) + (y_2-y_3)*(x_3-p_x)) * one_over_denom;
        b2 = ((p_y-y_1)*(x_3-x_1) + (y_3-y_1)*(x_1-p_x)) * one_over_denom;
        b3 = 1 - b1 - b2;

        result->x = b1;
        result->y = b2;
        result->z = b3;

        return true;
    }

    return false;
}

#if 0
bool32 bary_coords_inside_triangle(Vec3 bary_coords) {
    if (bary_coords.x >= 0 && bary_coords.x <= 1 &&
        bary_coords.y >= 0 && bary_coords.y <= 1 &&
        bary_coords.z >= 0 && bary_coords.z <= 1) {
        // inside
        return true;
    } else {
        return false;
    }
}
#else
bool32 bary_coords_inside_triangle(Vec3 bary_coords) {
    if (bary_coords.x >= -0.0001f && bary_coords.x <= 1.0001f &&
        bary_coords.y >= -0.0001f && bary_coords.y <= 1.0001f &&
        bary_coords.z >= -0.0001f && bary_coords.z <= 1.0001f) {
        // inside
        return true;
    } else {
        return false;
    }
}
#endif

inline bool32 are_collinear(Vec3 v1, Vec3 v2) {
    real32 dotted = dot(v1, v2);
    return (fabsf(1.0f - fabsf(dotted)) < EPSILON);
}

void make_basis(Vec3 v, Vec3 *right, Vec3 *up) {
    Vec3 axis_v;
    if (v.x < v.y) {
        if (v.x < v.z) {
            axis_v = x_axis;
        } else {
            axis_v = z_axis;
        }
    } else {
        axis_v = y_axis;
    }

    *right = normalize(cross(v, axis_v));
    *up = normalize(cross(v, *right));
}

// NOTE: this keeps forward the same. we're assuming that forward and right are already pretty close
//       to perpendicular, or we're just using this to ensure that they're orthogonal.
void orthonormalize(Vec3 forward, Vec3 right, Vec3 *new_forward, Vec3 *new_right) {
    Vec3 up = normalize(cross(forward, right));
    *new_right = normalize(cross(up, forward));
    *new_forward = normalize(forward);
}

// parallel_to_this is a vector we want the plane's normal to be closest to
Plane get_plane_containing_ray(Ray ray, Vec3 parallel_to_this) {
    Vec3 right, up;
    make_basis(ray.direction, &right, &up);

    real32 right_similarity = fabsf(dot(right, parallel_to_this));
    real32 up_similarity = fabsf(dot(up, parallel_to_this));

    Vec3 normal;
    real32 d;

    if (right_similarity > up_similarity) {
        normal = right;
    } else {
        normal = up;
    }

    d = dot(ray.origin, normal);

    Plane result = { d, normal };
    return result;
}

Plane get_plane_containing_ray(Ray ray) {
    Vec3 right, up;
    make_basis(ray.direction, &right, &up);
    real32 d = dot(ray.origin, right);

    Plane result = { d, right };
    return result;
}

// TODO: this can be further optimized, but it's fine for now (see scratchapixel for more optimizations)
bool32 ray_intersects_aabb(Ray ray, AABB aabb, real32 *t_min_result, real32 *t_max_result) {
    Vec3 origin = ray.origin;
    Vec3 direction = ray.direction;
    
    real32 x_min = aabb.p_min.x;
    real32 y_min = aabb.p_min.y;
    real32 z_min = aabb.p_min.z;
    real32 x_max = aabb.p_max.x;
    real32 y_max = aabb.p_max.y;
    real32 z_max = aabb.p_max.z;

    real32 t_min, t_max, t_y_min, t_y_max;

    if (direction.x >= 0) {
        t_min = (x_min - origin.x) / direction.x;
        t_max = (x_max - origin.x) / direction.x;
    } else {
        t_min = (x_max - origin.x) / direction.x;
        t_max = (x_min - origin.x) / direction.x;
    }

    if (direction.y >= 0) {
        t_y_min = (y_min - origin.y) / direction.y;
        t_y_max = (y_max - origin.y) / direction.y;
    } else {
        t_y_min = (y_max - origin.y) / direction.y;
        t_y_max = (y_min - origin.y) / direction.y;
    }
    
    if (t_min > t_y_max || t_y_min > t_max) return false;

    t_min = max(t_min, t_y_min);
    t_max = min(t_max, t_y_max);
    
    real32 t_z_min, t_z_max;
    if (direction.z >= 0) {
        t_z_min = (z_min - origin.z) / direction.z;
        t_z_max = (z_max - origin.z) / direction.z;
    } else {
        t_z_min = (z_max - origin.z) / direction.z;
        t_z_max = (z_min - origin.z) / direction.z;
    }

    if (t_min > t_z_max || t_z_min > t_max) return false;

    // test if origin is inside AABB
    // NOTE: this procedure actually returns true if the ray origin is inside the AABB without these checks,
    //       but the t_result will be negative. since we want to be able to select things when we're inside them,
    //       but not be able to select things that are behind us, we do this check.
    if (ray.origin.x >= aabb.p_min.x && ray.origin.y >= aabb.p_min.y && ray.origin.z >= aabb.p_min.z &&
        ray.origin.x <= aabb.p_max.x && ray.origin.y <= aabb.p_max.y && ray.origin.z <= aabb.p_max.z) {
        *t_min_result = 0.0f;
    } else {
        *t_min_result = max(t_min, t_z_min);
    }

    *t_max_result = min(t_max, t_z_max);
    
    return true;
}

inline bool32 ray_intersects_aabb(Ray ray, AABB aabb, real32 *t_result) {
    real32 t_max;
    return ray_intersects_aabb(ray, aabb, t_result, &t_max);
}

// NOTE: we assume ray.direction is a unit vector
//       if it were not, then we'd have to include the direction magnitude parts of the equation
// TODO: DON'T assume this, and instead use the equation that does not assume direction vector of unit length
bool32 ray_intersects_circle(Ray_2D ray, real32 radius, Vec2 center_pos, real32 *t_min, real32 *t_max) {
    // NOTE: these names, like delta don't really mean anything; it's just from the math
    //       see this page: https://www.geometrictools.com/Documentation/IntersectionLine2Circle2.pdf
    Vec2 delta = ray.origin - center_pos;

    real32 dir_dot_delta = dot(ray.direction, delta);
    real32 delta_distance_squared = dot(delta, delta);
    real32 r_squared = radius*radius;

    real32 direction_length = distance(ray.direction);
    real32 dir_squared = direction_length*direction_length;
    
    real32 to_sqrt = dir_dot_delta*dir_dot_delta - dir_squared*(delta_distance_squared - r_squared);

    if (to_sqrt < 0.0f) {
        return false;
    }

    real32 sqrted = sqrtf(to_sqrt);

    // TODO: maybe use epsilon value here
    if (direction_length == 0.0f) {
        if (delta_distance_squared < r_squared) {
            *t_min = -INFINITY;
            *t_max = INFINITY;
            return true;
        } else {
            return false;
        }
    }

    // NOTE: ray origin is inside circle; we do this check so that we don't get a negative t_min
    //       but, if we want to remove the if, we can just allow the negative t_min and make sure
    //       the caller checks for negative t_min, if necessary
    if (delta_distance_squared < r_squared) {
        *t_min = 0;
    } else {
        *t_min = (-dir_dot_delta - sqrted) / dir_squared;
    }
    *t_max = (-dir_dot_delta + sqrted) / dir_squared;
    return true;
}

bool32 ray_intersects_circle(Ray_2D ray, real32 radius, Vec2 center_pos, real32 *t_result) {
    real32 r_squared = radius*radius;
    Vec2 origin_to_center = center_pos - ray.origin;
    real32 origin_to_center_dist_squared = origin_to_center.x*origin_to_center.x + origin_to_center.y*origin_to_center.y;

    real32 origin_to_center_proj_direction = dot(origin_to_center, ray.direction);
    real32 origin_to_center_proj_direction_squared = origin_to_center_proj_direction*origin_to_center_proj_direction;

    real32 to_sqrt = r_squared - origin_to_center_dist_squared + origin_to_center_proj_direction_squared;
    
    // origin is inside circle
    if (origin_to_center_dist_squared < r_squared) {
        *t_result = 0;
        return true;
    }

    if (to_sqrt < 0) {
        return false;
    }
    
    *t_result = origin_to_center_proj_direction - sqrtf(to_sqrt);
    return true;
}

// NOTE: plane_normal is not necessarily a unit vector, thus plane_d is not necessarily the distance from the origin
//       of the plane
bool32 ray_intersects_plane(Ray ray, Vec3 plane_normal, real32 plane_d, real32 *t_result) {
    real32 denom = dot(ray.direction, plane_normal);
    if (fabsf(denom) < EPSILON) {
        return false;
    }
    
    real32 t = (plane_d - dot(ray.origin, plane_normal)) / denom;
    if (t >= 0.0f) {
        *t_result = (plane_d - dot(ray.origin, plane_normal)) / denom;
        return true;
    }

    return false;
}

bool32 bidirectional_ray_intersects_plane(Ray ray, Vec3 plane_normal, real32 plane_d, real32 *t_result) {
    real32 denom = dot(ray.direction, plane_normal);
    if (fabsf(denom) < EPSILON) {
        return false;
    }
    
    real32 t = (plane_d - dot(ray.origin, plane_normal)) / denom;
    *t_result = (plane_d - dot(ray.origin, plane_normal)) / denom;
    return true;
}

bool32 ray_intersects_plane(Ray ray, Plane plane, real32 *t_result) {
    return ray_intersects_plane(ray, plane.normal, plane.d, t_result);
}

inline real32 saturate(real32 t) {
    return min(max(t, 0.0f), 1.0f);
}

// NOTE: vertices should be in clockwise order
Vec3 get_triangle_normal(Vec3 triangle[3]) {
    Vec3 v1 = triangle[1] - triangle[0];
    Vec3 v2 = triangle[2] - triangle[1];
    Vec3 normal = normalize(cross(v1, v2));
    return normal;
}

Vec3 closest_point_on_line_segment(Vec3 a, Vec3 b, Vec3 point) {
    Vec3 AB = b - a;
    // we're pretty much dividing the dot product by the length of AB, to get the projected distance
    // of a over AB, i.e. dot(a, normalize(AB)), then dividing that projected distance by the distance of
    // AB, to get the percentage that the projected distance makes up of the length of AB.
    // so to do this we do
    // dot(point - a, AB) / ||AB|| / ||AB||
    // = dot(point - a, AB) / ||AB||^2
    // = dot(point - a, AB) / dot(AB, AB)
    real32 t = dot(point - a, AB) / dot(AB, AB);
    return a + saturate(t) * AB;
}

Vec3 get_closest_point_on_triangle_to_coplanar_point(Vec3 coplanar_point, Vec3 triangle[3], Vec3 triangle_normal) {
    // determine whether line_plane_intersection is inside the triangle
    Vec3 p0 = triangle[0];
    Vec3 p1 = triangle[1];
    Vec3 p2 = triangle[2];

    // TODO: we may need to do some checking here for degenerate triangles, i.e. where cross product would be
    //       the zero vector.
    Vec3 c0 = cross(coplanar_point - p0, p1 - p0);
    Vec3 c1 = cross(coplanar_point - p1, p2 - p1);
    Vec3 c2 = cross(coplanar_point - p2, p0 - p2);
    bool is_inside = (dot(c0, triangle_normal) <= 0.0f &&
                      dot(c1, triangle_normal) <= 0.0f &&
                      dot(c2, triangle_normal) <= 0.0f);
 
    if (is_inside)  {
        return coplanar_point;
    } else {
        // find the closest points on the triangle edges to the point that lies outside the triangle
        // and we return the closest one found

        // edge 1
        Vec3 point1 = closest_point_on_line_segment(p0, p1, coplanar_point);
        Vec3 v1 = coplanar_point - point1;
        real32 dist_squared = dot(v1, v1);
        real32 smallest_distance = dist_squared;
        Vec3 closest_point = point1;
 
        // edge 2
        Vec3 point2 = closest_point_on_line_segment(p1, p2, coplanar_point);
        Vec3 v2 = coplanar_point - point2;
        dist_squared = dot(v2, v2);
        if(dist_squared < smallest_distance) {
            closest_point = point2;
            smallest_distance = dist_squared;
        }
 
        // edge 3
        Vec3 point3 = closest_point_on_line_segment(p2, p0, coplanar_point);
        Vec3 v3 = coplanar_point - point3;
        dist_squared = dot(v3, v3);
        if(dist_squared < smallest_distance) {
            closest_point = point3;
            smallest_distance = dist_squared;
        }

        return closest_point;
    }
}

Vec3 get_point_on_plane_from_xz(real32 x, real32 z, Vec3 plane_normal, Vec3 some_point_on_plane) {
    Vec3 n = normalize(plane_normal);
    real32 plane_d = dot(some_point_on_plane, n);

    // NOTE: this is the case where the plane is just a straight wall; we don't support this case
    assert(fabsf(n.y) > EPSILON);

    real32 projected_y = (plane_d - n.x*x - n.z*z) / n.y;
    return make_vec3(x, projected_y, z);
}

bool32 circle_intersects_triangle_on_xz_plane(Vec3 center, real32 radius, Vec3 triangle[3], Vec3 triangle_normal) {
    Vec3 p0 = triangle[0];
    Vec3 p1 = triangle[1];
    Vec3 p2 = triangle[2];

    Vec3 projected_triangle[] = {
        make_vec3(p0.x, 0.0f, p0.z),
        make_vec3(p1.x, 0.0f, p1.z),
        make_vec3(p2.x, 0.0f, p2.z)
    };
    
    Vec3 projected_center = make_vec3(center.x, 0.0f, center.z);

    Vec3 projected_triangle_normal = normalize(make_vec3(0.0f, triangle_normal.y, 0.0f));
    Vec3 closest_point_on_triangle = get_closest_point_on_triangle_to_coplanar_point(projected_center,
                                                                                     projected_triangle,
                                                                                     projected_triangle_normal);
    Vec3 point_to_center = projected_center - closest_point_on_triangle;
    real32 radius_squared = radius*radius;
    if (dot(point_to_center, point_to_center) > radius_squared) {
        // circle doesn't touch the projected triangle
        return false;
    }

    return true;
}

bool32 sphere_intersects_triangle(Vec3 center, real32 radius, Vec3 triangle[3],
                                  Vec3 *penetration_normal, real32 *penetration_depth) {
    Vec3 p0 = triangle[0];
    Vec3 p1 = triangle[1];
    Vec3 p2 = triangle[2];
    
    Vec3 triangle_normal = get_triangle_normal(triangle);

    // check if the sphere intersects the plane containing the triangle
    real32 center_distance_from_plane = dot(center - p0, triangle_normal);
    if (fabsf(center_distance_from_plane) > radius) return false;

    Vec3 coplanar_point = center - triangle_normal*center_distance_from_plane;
    Vec3 closest_point_on_triangle = get_closest_point_on_triangle_to_coplanar_point(coplanar_point, triangle,
                                                                                     triangle_normal);
    Vec3 point_to_center = center - closest_point_on_triangle;
    real32 radius_squared = radius*radius;
    if (dot(point_to_center, point_to_center) > radius_squared) return false;

    Vec3 penetration_vector = center - closest_point_on_triangle;
    *penetration_normal = normalize(penetration_vector);
    *penetration_depth = radius - distance(penetration_vector);

    Vec3 capsule_edge = closest_point_on_triangle - (*penetration_normal)*(*penetration_depth);

#if DEBUG_SHOW_COLLISION_LINES
    add_debug_line(&Context::game_state->debug_state,
                   closest_point_on_triangle, capsule_edge, make_vec4(1.0f, 1.0f, 0.0f, 1.0f));
#endif

#if 0
    if (dot(penetration_vector, triangle_normal) > 0.0f) {
        return true;
    }

    return false;
#endif
    return true;
}

// https://wickedengine.net/2020/04/26/capsule-collision-detection/
bool32 capsule_intersects_triangle(Capsule capsule, Vec3 triangle[3],
                                   Vec3 *penetration_normal, real32 *penetration_depth,
                                   Vec3 *intersection_point) {
    Vec3 capsule_normal = normalize(capsule.tip - capsule.base);
    Vec3 line_end_offset = capsule_normal * capsule.radius;
    Vec3 a = capsule.base + line_end_offset;
    Vec3 b = capsule.tip - line_end_offset;

    Vec3 triangle_normal = get_triangle_normal(triangle);
    Ray capsule_ray = make_ray(capsule.base, capsule_normal);
    real32 plane_d = dot(triangle[0], triangle_normal);
    real32 plane_intersect_t;

    Vec3 reference_point;
    if (bidirectional_ray_intersects_plane(capsule_ray, triangle_normal, plane_d, &plane_intersect_t)) {
        Vec3 plane_intersection_point = capsule.base + capsule_normal * plane_intersect_t;
        reference_point = get_closest_point_on_triangle_to_coplanar_point(plane_intersection_point,
                                                                          triangle, triangle_normal);
    } else {
        reference_point = triangle[0];
    }
    
    // the penetration normal is the direction vector of the shortest line from the triangle to the reference
    // point. the penetration vector is NOT the distance we need to move the capsule by to get it out. the
    // direction is the same, but the distance is the radius of the capsule - the length of the penetration vector.
    Vec3 sphere_center = closest_point_on_line_segment(a, b, reference_point);
    if (!sphere_intersects_triangle(sphere_center, capsule.radius, triangle,
                                    penetration_normal, penetration_depth)) {
        return false;
    }

#if 0
    //DEBUG_SHOW_COLLISION_LINES
    add_debug_line(&Context::game_state->debug_state,
                   triangle[0], triangle[1], make_vec4(1.0f, 0.0f, 0.0f, 1.0f));
    add_debug_line(&Context::game_state->debug_state,
                   triangle[1], triangle[2], make_vec4(1.0f, 0.0f, 0.0f, 1.0f));
    add_debug_line(&Context::game_state->debug_state,
                   triangle[2], triangle[0], make_vec4(1.0f, 0.0f, 0.0f, 1.0f));
#endif

    *intersection_point = sphere_center - (*penetration_normal)*capsule.radius;

    return true;
}

// TODO: replace this with the faster ray vs triangle test
//       (ctrl-f triangle here: https://www.iquilezles.org/www/articles/intersectors/intersectors.htm)
//       detailed explanation of fast algorithm here: https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
// TODO: may also be able to do a ray vs many triangles SIMD version of this procedure
#if 0
bool32 ray_intersects_triangle(Ray ray, Vec3 v[3], real32 *t_result) {
    Vec3 v0_v1 = v[1] - v[0];
    Vec3 v0_v2 = v[2] - v[0];

    Vec3 n = cross(v0_v1, v0_v2);
    real32 d = dot(n, v[0]); // this can be negative

    real32 denom = dot(ray.direction, n);
    // weird check to bail on NaN
#if 0
    if (!(denom < 0.0f)) {
        // ray is hitting backside of triangle
        return false;
    }
#endif
    if (fabs(denom) < EPSILON) {
        // ray is parallel to triangle
        return false;
    }

    real32 t = (d - dot(ray.origin, n)) / denom;

    if (t < 0.0f) return false;

    Vec3 intersection_point = ray.origin + t*ray.direction;
    Vec3 bary_coords;
    bool32 triangle_is_valid = compute_barycentric_coords(v[0], v[1], v[2],
                                                          n, intersection_point,
                                                          &bary_coords);

    if (!triangle_is_valid) return false;

    bool32 inside_triangle = bary_coords_inside_triangle(bary_coords);
    if (inside_triangle) {
        *t_result = t;
        return true;
    }

    return false;
}
#endif

// from https://www.iquilezles.org/www/articles/intersectors/intersectors.htm
// explanation (i'm pretty sure this is just a slightly modified version of the above) here:
// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
bool32 ray_intersects_triangle(Ray ray, Vec3 triangle_verts[3], bool32 include_backside, real32 *t_result) {
    Vec3 v1v0 = triangle_verts[1] - triangle_verts[0];
    Vec3 v2v0 = triangle_verts[2] - triangle_verts[0];
    Vec3 rov0 = ray.origin - triangle_verts[0];
    Vec3 n = cross(v1v0, v2v0);
    Vec3 q = cross(rov0, ray.direction);

    real32 denom = dot(ray.direction, n);
    if (!include_backside) {
        // check if ray is hitting backside of triangle
        if (!(denom < 0.0f)) return false;
    }

    if (fabsf(denom) < EPSILON) return false;
    
    real32 d = 1.0f / denom;
    real32 u = d * dot(-q, v2v0);
    real32 v = d * dot( q, v1v0);
    real32 t = d * dot(-n, rov0);
    
    if (u < 0.0f || v < 0.0f || (u + v) > 1.0 || t < 0.0f) return false;

    *t_result = t;
    return true;
}

// NOTE: we note that it's coplanar because this doesn't check for skew lines
//       checking for skew lines would require us to find both t_1 and t_2
#if 0
bool32 line_intersects_coplanar_ray(Line line, Ray ray, real32 *t_result) {
    Vec3 p1 = line.origin;
    Vec3 p2 = ray.origin;
    Vec3 d1 = line.line;
    Vec3 d2 = ray.direction;

    real32 d1_cross_d2_distance = distance(cross(d1, d2));
    real32 denom = d1_cross_d2_distance * d1_cross_d2_distance;

    // check if lines are parellel
    if (fabsf(denom) < EPSILON) return false;

    real32 t = dot(cross((p2 - p1), d2), cross(d1, d2)) / denom;
    // make sure the t is within the line segment
    if (t > -0.001f && t < 1.001f) {
        *t_result = t;
        return true;
    } else {
        return false;
    }
}
#endif

bool32 ray_intersects_coplanar_ray(Ray ray1, Ray ray2, real32 *t_result) {
    Vec3 p1 = ray1.origin;
    Vec3 p2 = ray2.origin;
    Vec3 d1 = ray1.direction;
    Vec3 d2 = ray2.direction;

    real32 d1_cross_d2_distance = distance(cross(d1, d2));
    real32 denom = d1_cross_d2_distance * d1_cross_d2_distance;

    // check if lines are parallel
    if (fabsf(denom) < EPSILON) return false;

    real32 t = dot(cross((p2 - p1), d2), cross(d1, d2)) / denom;
    //if (t < 0.0f) return false;

    *t_result = t;
    return true;
}

// TODO: optimize probably
// NOTE: this doesn't calculate a tight AABB.
//       it just transforms the original AABB and gives you the new AABB for that transformed AABB.
//       the way this function does it is more efficient than transforming all 8 points and
//       creating an AABB from that.
void transform_aabb(AABB aabb, Mat4 transform_matrix, AABB *transformed_aabb) {
    Vec3 new_origin = truncate_v4_to_v3(transform_matrix.col4);

    real32 x_min = new_origin.x;
    real32 x_max = new_origin.x;
    real32 y_min = new_origin.y;
    real32 y_max = new_origin.y;
    real32 z_min = new_origin.z;
    real32 z_max = new_origin.z;

    if (transform_matrix.col1[0] > 0) {
        x_min += transform_matrix.col1[0] * aabb.p_min.x;
        x_max += transform_matrix.col1[0] * aabb.p_max.x;
    } else {
        x_min += transform_matrix.col1[0] * aabb.p_max.x;
        x_max += transform_matrix.col1[0] * aabb.p_min.x;
    }

    if (transform_matrix.col1[1] > 0) {
        y_min += transform_matrix.col1[1] * aabb.p_min.x;
        y_max += transform_matrix.col1[1] * aabb.p_max.x;
    } else {
        y_min += transform_matrix.col1[1] * aabb.p_max.x;
        y_max += transform_matrix.col1[1] * aabb.p_min.x;
    }
    
    if (transform_matrix.col1[2] > 0) {
        z_min += transform_matrix.col1[2] * aabb.p_min.x;
        z_max += transform_matrix.col1[2] * aabb.p_max.x;
    } else {
        z_min += transform_matrix.col1[2] * aabb.p_max.x;
        z_max += transform_matrix.col1[2] * aabb.p_min.x;
    }

    if (transform_matrix.col2[0] > 0) {
        x_min += transform_matrix.col2[0] * aabb.p_min.y;
        x_max += transform_matrix.col2[0] * aabb.p_max.y;
    } else {
        x_min += transform_matrix.col2[0] * aabb.p_max.y;
        x_max += transform_matrix.col2[0] * aabb.p_min.y;
    }

    if (transform_matrix.col2[1] > 0) {
        y_min += transform_matrix.col2[1] * aabb.p_min.y;
        y_max += transform_matrix.col2[1] * aabb.p_max.y;
    } else {
        y_min += transform_matrix.col2[1] * aabb.p_max.y;
        y_max += transform_matrix.col2[1] * aabb.p_min.y;
    }

    if (transform_matrix.col2[2] > 0) {
        z_min += transform_matrix.col2[2] * aabb.p_min.y;
        z_max += transform_matrix.col2[2] * aabb.p_max.y;
    } else {
        z_min += transform_matrix.col2[2] * aabb.p_max.y;
        z_max += transform_matrix.col2[2] * aabb.p_min.y;
    }
    
    if (transform_matrix.col3[0] > 0) {
        x_min += transform_matrix.col3[0] * aabb.p_min.z;
        x_max += transform_matrix.col3[0] * aabb.p_max.z;
    } else {
        x_min += transform_matrix.col3[0] * aabb.p_max.z;
        x_max += transform_matrix.col3[0] * aabb.p_min.z;
    }
    
    if (transform_matrix.col3[1] > 0) {
        y_min += transform_matrix.col3[1] * aabb.p_min.z;
        y_max += transform_matrix.col3[1] * aabb.p_max.z;
    } else {
        y_min += transform_matrix.col3[1] * aabb.p_max.z;
        y_max += transform_matrix.col3[1] * aabb.p_min.z;
    }

    if (transform_matrix.col3[2] > 0) {
        z_min += transform_matrix.col3[2] * aabb.p_min.z;
        z_max += transform_matrix.col3[2] * aabb.p_max.z;
    } else {
        z_min += transform_matrix.col3[2] * aabb.p_max.z;
        z_max += transform_matrix.col3[2] * aabb.p_min.z;
    }
    
    transformed_aabb->p_min = make_vec3(x_min, y_min, z_min);
    transformed_aabb->p_max = make_vec3(x_max, y_max, z_max);
}

AABB transform_aabb(AABB aabb, Mat4 transform_matrix) {
    AABB transformed_aabb = {};
    Vec3 new_origin = truncate_v4_to_v3(transform_matrix.col4);

    real32 x_min = new_origin.x;
    real32 x_max = new_origin.x;
    real32 y_min = new_origin.y;
    real32 y_max = new_origin.y;
    real32 z_min = new_origin.z;
    real32 z_max = new_origin.z;

    if (transform_matrix.col1[0] > 0) {
        x_min += transform_matrix.col1[0] * aabb.p_min.x;
        x_max += transform_matrix.col1[0] * aabb.p_max.x;
    } else {
        x_min += transform_matrix.col1[0] * aabb.p_max.x;
        x_max += transform_matrix.col1[0] * aabb.p_min.x;
    }

    if (transform_matrix.col1[1] > 0) {
        y_min += transform_matrix.col1[1] * aabb.p_min.x;
        y_max += transform_matrix.col1[1] * aabb.p_max.x;
    } else {
        y_min += transform_matrix.col1[1] * aabb.p_max.x;
        y_max += transform_matrix.col1[1] * aabb.p_min.x;
    }
    
    if (transform_matrix.col1[2] > 0) {
        z_min += transform_matrix.col1[2] * aabb.p_min.x;
        z_max += transform_matrix.col1[2] * aabb.p_max.x;
    } else {
        z_min += transform_matrix.col1[2] * aabb.p_max.x;
        z_max += transform_matrix.col1[2] * aabb.p_min.x;
    }

    if (transform_matrix.col2[0] > 0) {
        x_min += transform_matrix.col2[0] * aabb.p_min.y;
        x_max += transform_matrix.col2[0] * aabb.p_max.y;
    } else {
        x_min += transform_matrix.col2[0] * aabb.p_max.y;
        x_max += transform_matrix.col2[0] * aabb.p_min.y;
    }

    if (transform_matrix.col2[1] > 0) {
        y_min += transform_matrix.col2[1] * aabb.p_min.y;
        y_max += transform_matrix.col2[1] * aabb.p_max.y;
    } else {
        y_min += transform_matrix.col2[1] * aabb.p_max.y;
        y_max += transform_matrix.col2[1] * aabb.p_min.y;
    }

    if (transform_matrix.col2[2] > 0) {
        z_min += transform_matrix.col2[2] * aabb.p_min.y;
        z_max += transform_matrix.col2[2] * aabb.p_max.y;
    } else {
        z_min += transform_matrix.col2[2] * aabb.p_max.y;
        z_max += transform_matrix.col2[2] * aabb.p_min.y;
    }
    
    if (transform_matrix.col3[0] > 0) {
        x_min += transform_matrix.col3[0] * aabb.p_min.z;
        x_max += transform_matrix.col3[0] * aabb.p_max.z;
    } else {
        x_min += transform_matrix.col3[0] * aabb.p_max.z;
        x_max += transform_matrix.col3[0] * aabb.p_min.z;
    }
    
    if (transform_matrix.col3[1] > 0) {
        y_min += transform_matrix.col3[1] * aabb.p_min.z;
        y_max += transform_matrix.col3[1] * aabb.p_max.z;
    } else {
        y_min += transform_matrix.col3[1] * aabb.p_max.z;
        y_max += transform_matrix.col3[1] * aabb.p_min.z;
    }

    if (transform_matrix.col3[2] > 0) {
        z_min += transform_matrix.col3[2] * aabb.p_min.z;
        z_max += transform_matrix.col3[2] * aabb.p_max.z;
    } else {
        z_min += transform_matrix.col3[2] * aabb.p_max.z;
        z_max += transform_matrix.col3[2] * aabb.p_min.z;
    }
    
    transformed_aabb.p_min = make_vec3(x_min, y_min, z_min);
    transformed_aabb.p_max = make_vec3(x_max, y_max, z_max);

    return transformed_aabb;
}

inline AABB transform_aabb(AABB aabb, Transform transform) {
    return transform_aabb(aabb, get_model_matrix(transform));
}

inline bool32 is_zero(Vec3 v) {
    return (v.x == 0 && v.y == 0 && v.z == 0);
}

inline bool32 is_zero(Vec4 v) {
    return (v.x == 0 && v.y == 0 && v.z == 0 && v.w == 0);
}

Mat4 get_rotate_matrix_from_euler_angles(real32 roll, real32 pitch, real32 heading) {
    Mat4 model_matrix = make_mat4_identity();

    // z, x, y rotation order (blue, red, green)
    model_matrix = make_rotate_matrix(z_axis, roll) * model_matrix;
    model_matrix = make_rotate_matrix(x_axis, pitch) * model_matrix;
    model_matrix = make_rotate_matrix(y_axis, heading) * model_matrix;
    
    return model_matrix;
}

void get_euler_angles_from_rotate_matrix(Mat4 rotate_matrix, real32 *canonical_roll, real32 *canonical_pitch, real32 *canonical_heading) {
    real32 heading, pitch, roll;

    real32 m23 = rotate_matrix.col3[1];
    real32 sin_pitch = -m23;

    real32 m13 = rotate_matrix.col3[0];
    real32 m11 = rotate_matrix.col1[0];

    pitch = asinf(sin_pitch);
    
    // NOTE: this is the gimbal lock case - pitch is very close to -90 or +90 degrees
    if (fabsf(sin_pitch) > 0.99999f) {
        roll = 0.0f;
        heading = atan2f(-m13, m11);
    } else {
        real32 m33 = rotate_matrix.col3[2];
        heading = (real32) atan2(m13, m33);

        real32 m21 = rotate_matrix.col1[1];
        real32 m22 = rotate_matrix.col2[1];
        roll = (real32) atan2(m21, m22);
    }

    *canonical_heading = heading;
    *canonical_pitch = pitch;
    *canonical_roll = roll;
}

void get_euler_angles_from_quaternion(Quaternion q, Vec3 *result) {
    Mat4 rotate_matrix = make_rotate_matrix(q);
    // roll    = z-axis
    // pitch   = x-axis
    // heading = y-axis
    get_euler_angles_from_rotate_matrix(rotate_matrix, &result->z, &result->x, &result->y);
}

Mat4 get_model_matrix(Vec3 scale, Quaternion rotation, Vec3 position) {
    Mat4 model_matrix = make_mat4_identity();

    model_matrix = make_scale_matrix(scale) * model_matrix;
    model_matrix = make_rotate_matrix(rotation) * model_matrix;
    model_matrix = make_translate_matrix(position) * model_matrix;

    // TODO: maybe don't do a copy here, and just use an output parameter
    return model_matrix;
}

Mat4 get_model_matrix(Vec3 scale, real32 roll, real32 pitch, real32 heading, Vec3 position) {
    Mat4 model_matrix = make_mat4_identity();

    // transform the mesh
    // scale
    model_matrix = make_scale_matrix(scale) * model_matrix;
    // rotation
    model_matrix = make_rotate_matrix(z_axis, roll) * model_matrix;
    model_matrix = make_rotate_matrix(x_axis, pitch) * model_matrix;
    model_matrix = make_rotate_matrix(y_axis, heading) * model_matrix;
    // position
    model_matrix = make_translate_matrix(position) * model_matrix;

    // TODO: maybe don't do a copy here, and just use an output parameter
    return model_matrix;
}

Mat4 get_model_matrix(Transform transform) {
    return get_model_matrix(transform.scale,
                            transform.rotation,
                            transform.position);
}

Mat4 get_model_matrix(Euler_Transform transform) {
    return get_model_matrix(transform.scale,
                            transform.roll, transform.pitch, transform.heading,
                            transform.position);
}

inline real32 cosine_law_degrees(real32 a, real32 b, real32 c) {
#if 0
    if (a + c <= b) {
        return 0.0f;
    }
#endif
    real32 radians = acosf((c*c - a*a - b*b) / (-2.0f * a * b));
    if (isnan(radians)) return 0.0f; // not sure if this is good or not, but it works
    return rads_to_degs(radians);
}

Vec3 compute_centroid(Ray side1_ray, Ray side2_ray, Ray side3_ray) {
    real32 denom = 1.0f / 3.0f;
    Vec3 centroid =  denom * (side1_ray.origin + side2_ray.origin + side3_ray.origin);
    return centroid;
}

// NOTE: returns the point on the ray that's closest to the point
Vec3 get_closest_point_on_ray(Ray ray, Vec3 point) {
    Vec3 unit_direction = normalize(ray.direction);
    Vec3 closest_point_on_ray = ray.origin + unit_direction*dot(point - ray.origin, unit_direction);
    return closest_point_on_ray;
}

Vec3 get_ray_normal_to_point(Ray ray, Vec3 point) {
    Vec3 closest_point_on_ray = get_closest_point_on_ray(ray, point);
    assert(distance(point - closest_point_on_ray) > 0.000001f);
    return normalize(point - closest_point_on_ray);
}

Vec4 rgba_to_vec4(real32 r, real32 g, real32 b, real32 a) {
    real32 denom = 1.0f / 255.0f;
    return make_vec4(denom*r, denom*g, denom*b, denom*a);
}

Vec4 rgb_to_vec4(real32 r, real32 g, real32 b) {
    real32 denom = 1.0f / 255.0f;
    return make_vec4(denom*r, denom*g, denom*b, 1.0f);
}

Vec4 rgb_to_vec4(RGB_Color rgb) {
    real32 denom = 1.0f / 255.0f;
    return make_vec4(denom*rgb.r, denom*rgb.g, denom*rgb.b, 1.0f);
}

Vec4 rgb_to_vec4(int32 r, int32 g, int32 b) {
    real32 denom = 1.0f / 255.0f;
    return make_vec4(denom*r, denom*g, denom*b, 1.0f);
}

real32 clamp(real32 value, real32 min, real32 max) {
    value = min(value, max);
    value = max(value, min);
    return value;
}

int32 clamp(int32 value, int32 min, int32 max) {
    value = min(value, max);
    value = max(value, min);
    return value;
}

real32 mix(real32 a, real32 b, real32 t) {
    return a*(1.0f - t) + b*t;
}

Vec3 mix(Vec3 a, Vec3 b, real32 t) {
    return a*(1.0f - t) + b*t;
}

Vec4 mix(Vec4 a, Vec4 b, real32 t) {
    return a*(1.0f - t) + b*t;
}

RGB_Color vec3_to_rgb(Vec3 v) {
    RGB_Color result;
    real32 min = 0.0f;
    real32 max = 255.0f;
    result.r = clamp(v.x * max, min, max);
    result.g = clamp(v.y * max, min, max);
    result.b = clamp(v.z * max, min, max);
    return result;
}

Vec3 rgb_to_vec3(RGB_Color rgb) {
    Vec3 result;
    real32 min = 0.0f;
    real32 max = 1.0f;
    real32 max_rgb = 255.0f;
    result.x = clamp(rgb.r / max_rgb, min, max);
    result.y = clamp(rgb.g / max_rgb, min, max);
    result.z = clamp(rgb.b / max_rgb, min, max);
    return result;
}

RGB_Color hsv_to_rgb(HSV_Color hsv_color) {
    assert(hsv_color.h >= 0.0f && hsv_color.h <= 360.0f);
    assert(hsv_color.s >= 0.0f && hsv_color.s <= 100.0f);
    assert(hsv_color.v >= 0.0f && hsv_color.v <= 100.0f);

    Vec3 colors[6] = {
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 0.0f, 1.0f }
    };

    // TODO: this might be wrong
    int segment = (int32) (hsv_color.h / 60.0f) % 6;
    float segment_percentage = (hsv_color.h / 60.0f) - (int32) (hsv_color.h / 60.0f);

    Vec3 color1 = colors[segment];
    Vec3 color2 = colors[(segment + 1) % 6];
    
    Vec3 hue = mix(color1, color2, segment_percentage);
    Vec3 white = make_vec3(1.0f, 1.0f, 1.0f);
    Vec3 black = make_vec3();

    Vec3 v = mix(white, hue, (real32) hsv_color.s / 100.0f);
    v = mix(black, v, (real32) hsv_color.v / 100.0f );
    // or v = v * ((real32) hsv_color.v / 100.0f)

    return vec3_to_rgb(v);
}

HSV_Color rgb_to_hsv(RGB_Color rgb_color) {
    int32 segment = 0;
    real32 max_value, min_value;
    real32 red = rgb_color.r;
    real32 green = rgb_color.g;
    real32 blue = rgb_color.b;

    assert (red >= 0.0f && red <= 255.0f);
    assert (green >= 0.0f && green <= 255.0f);
    assert (blue >= 0.0f && blue <= 255.0f);

    if ((red == green) && (green == blue)) {
        segment = 0;
        max_value = red;
        min_value = red;
    } else if ((red > green) && (red > blue)) {
        segment = 1;
        max_value = red;
        min_value = (green < blue) ? green : blue;
    } else if ((green > red) && (green > blue)) {
        segment = 2;
        max_value = green;
        min_value = (red < blue) ? red : blue;
    } else {
        segment = 3;
        max_value = blue;
        min_value = (red < green) ? red : green;
    }

    real32 hue = 0.0f;
    real32 range = max_value - min_value;
    switch (segment) {
        case 0: {
            hue = 0.0f;
        } break;
        case 1: {
            hue = 60.0f * ((green - blue) / range);
        } break;
        case 2: {
            hue = 60.0f * (2.0f + (blue - red) / range);
        } break;
        case 3: {
            hue = 60.0f * (4.0f + (red - green) / range);
        } break;
    };

    if (hue < 0) {
        hue += 360;
    }

    real32 saturation = (segment == 0) ? 0 : (range / max_value);
    saturation = saturation * 100.0f;
    saturation = clamp(saturation, 0.0f, 100.0f);

    real32 value = max_value / 255.0f * 100.0f;
    value = clamp(value, 0.0f, 100.0f);

    hue = clamp(hue, 0.0f, 360.0f);

    return { hue, saturation, value };
}

#if 0
real32 get_z_on_triangle(real32 x, real32 y, Vec3 triangle[3]) {
    Vec3 u = triangle[1] - triangle[0];
    Vec3 v = triangle[2] - triangle[0];

    // just flatten it
    u.z = 0.0f;
    v.z = 0.0f;
    
    Vec3 point = make_vec3(x, y, 0.0f);
    Vec3 origin_to_point = normalize(point - triangle[0]);
}
#endif

#if 0
bool32 in_triangle(Vec3 p, Vec3 basis1, Vec3 basis2) {
    real32 u = dot(p, normalize(basis1) / distance(basis1));
    real32 v = dot(p, normalize(basis2) / distance(basis2));

    if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f || (u + v) > 1.0f) {
        return false;
    } else {
        return true;
    }
}
#endif

bool32 in_triangle(Vec2 p, Vec2 triangle[3]) {
    real32 x1 = triangle[0].x;
    real32 y1 = triangle[0].y;
    real32 x2 = triangle[1].x;
    real32 y2 = triangle[1].y;
    real32 x3 = triangle[2].x;
    real32 y3 = triangle[2].y;
    
    real32 denom = (y1-y3)*(x2-x3) + (y2-y3)*(x3-x1);
    // check for triangle of zero area
    if (fabs(denom) < EPSILON) return false;

    real32 one_over_denom = 1.0f / denom;

    real32 b1 = one_over_denom * ((p.y-y3)*(x2-x3) + (y2-y3)*(x3-p.x));
    real32 b2 = one_over_denom * ((p.y-y1)*(x3-x1) + (y3-y1)*(x1-p.x));
    real32 b3 = 1 - b1 - b2;

    if (b1 < 0.0f || b2 < 0.0f || b3 < 0.0f || (b1 + b2 + b3) > 1.0f) {
        return false;
    }

    return true;
}

// NOTE: gets the point on a triangle that a vertical line passing through the xz-plane hits
// NOTE: xz is our flat plane in our coordinate-space, and so where usually you would see y, is now z
// NOTE: we pass in x and z instead of a Vec2, since Vec2 has members x and y, and i thought it looked
//       confusing when using them in the equations.
bool32 get_point_on_triangle_from_xz(real32 x, real32 z, Vec3 triangle[3], Vec3 *point_on_triangle) {
    // drop out y-coordinate
    real32 x1 = triangle[0].x;
    real32 z1 = triangle[0].z;
    real32 x2 = triangle[1].x;
    real32 z2 = triangle[1].z;
    real32 x3 = triangle[2].x;
    real32 z3 = triangle[2].z;

    real32 denom = (z1-z3)*(x2-x3) + (z2-z3)*(x3-x1);
    // check for triangle of zero area
    if (fabs(denom) < EPSILON) return false;

    real32 one_over_denom = 1.0f / denom;

    real32 b1 = one_over_denom * ((z-z3)*(x2-x3) + (z2-z3)*(x3-x));
    real32 b2 = one_over_denom * ((z-z1)*(x3-x1) + (z3-z1)*(x1-x));
    real32 b3 = 1 - b1 - b2;

    if (b1 < 0.0f || b2 < 0.0f || b3 < 0.0f || (b1 + b2 + b3) > 1.0f) {
        return false;
    }

    *point_on_triangle = b1*triangle[0] + b2*triangle[1] + b3*triangle[2];
    return true;
}

inline Vec3 transform_point(Mat4 *model_matrix, Vec3 *point) {
    return truncate_v4_to_v3(*model_matrix * make_vec4(*point, 1.0f));
}

inline void transform_triangle(Vec3 triangle[3], Mat4 *model_matrix) {
    for (int32 i = 0; i < 3; i++) {
        triangle[i] = transform_point(model_matrix, &triangle[i]);
    }
}

