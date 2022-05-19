#ifndef RENDER_H
#define RENDER_H

struct Camera {
    real32 fov_x_degrees;
    real32 aspect_ratio;
    real32 near;
    real32 far;
    
    real32 heading;
    real32 pitch;
    real32 roll;

    // NOTE: direction and right must be unit vectors and orthogonal
    Basis initial_basis;
    Basis current_basis;
    Vec3 position;
};

struct Display_Output {
    int32 width;
    int32 height;
};

struct Render_State {
    Display_Output display_output;
    Camera camera;
    Mat4 perspective_clip_matrix;
    Mat4 view_matrix;
    Mat4 cpv_matrix;
    Mat4 ortho_clip_matrix;
};

#endif
