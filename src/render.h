#ifndef RENDER_H
#define RENDER_H

#define MAX_COMMANDS 256

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

enum class Command_Type {
    LOAD_FONT,
    LOAD_MESH, UNLOAD_MESH,
    LOAD_TEXTURE, UNLOAD_TEXTURE
};

struct Command_Load_Mesh {
    String mesh_name;
};

struct Command_Unload_Mesh {
    String mesh_name;
};

struct Command {
    Command_Type type;
    union {
        Command_Load_Mesh load_mesh;
        Command_Unload_Mesh unload_mesh;
    };
};

struct Command_Queue {
    // commands are per frame
    int32 num_commands;
    Command commands[MAX_COMMANDS];
};

struct Render_State {
    Display_Output display_output;
    Camera camera;
    Mat4 perspective_clip_matrix;
    Mat4 view_matrix;
    Mat4 cpv_matrix;
    Mat4 ortho_clip_matrix;

    Command_Queue command_queue;
};

#endif
