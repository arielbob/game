#ifndef GAME_H
#define GAME_H

#include "ui.h"
#include "mesh.h"

struct Display_Output {
    int32 width;
    int32 height;
};

struct Sound_Output {
    int16 *sound_buffer;
    uint32 buffer_size;
    uint32 max_samples;
    uint32 samples_per_second;
};

// NOTE: this is assumed to be at same sample rate and bit depth as the initialized directsound buffers
struct Audio_Source {
    int32 total_samples;
    int32 current_sample;
    real32 volume;
    bool32 should_loop;
    int16 *samples;
};

struct Controller_Button_State {
    bool32 was_down;
    bool32 is_down;
};

struct Controller_State {
    // keyboard controls
    // NOTE: not really suited for rebinding currently
    // TODO: look into supporting other keyboard layouts

    char pressed_key = '\0';
    union {
        Controller_Button_State key_states[15];
        struct {
            Controller_Button_State key_shift;
            Controller_Button_State key_ctrl;
            Controller_Button_State key_alt;
            Controller_Button_State key_w;
            Controller_Button_State key_a;
            Controller_Button_State key_s;
            Controller_Button_State key_d;
            Controller_Button_State key_e;
            Controller_Button_State key_up;
            Controller_Button_State key_down;
            Controller_Button_State key_right;
            Controller_Button_State key_left;
            Controller_Button_State left_mouse;
            Controller_Button_State right_mouse;
            Controller_Button_State middle_mouse;
        };
    };
    
    // gamepad controls
    Controller_Button_State up;
    Controller_Button_State down;
    Controller_Button_State right;
    Controller_Button_State left;
    Controller_Button_State start;
    Controller_Button_State back;
    Controller_Button_State left_thumb;
    Controller_Button_State right_thumb;
    Controller_Button_State left_shoulder;
    Controller_Button_State right_shoulder;
    Controller_Button_State a;
    Controller_Button_State b;
    Controller_Button_State x;
    Controller_Button_State y;

    real32 left_thumb_x;
    real32 left_thumb_y;
    real32 right_thumb_x;
    real32 right_thumb_y;

    Vec2 last_mouse;
    Vec2 current_mouse;
};

struct Camera {
    real32 fov_x_degrees;
    real32 aspect_ratio;
    real32 near;
    real32 far;
    
    real32 heading;
    real32 pitch;
    real32 roll;

    // NOTE: direction and right must be unit vectors and orthogonal
    Vec3 forward;
    Vec3 right;
    Vec3 position;
};

struct Render_State {
    Camera camera;
    Mat4 cpv_matrix;
};

#define MAX_MESHES 64

struct Game_State {
    bool32 is_initted;
    Render_State render_state;
    Audio_Source music;
    bool32 is_playing_music;
    UI_Manager ui_manager;
    char current_char; // debugging
    Vec2 cursor_pos; // debugging

    int32 num_meshes;
    Mesh meshes[MAX_MESHES];
};

#endif
