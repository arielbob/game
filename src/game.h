#ifndef GAME_H
#define GAME_H

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

struct Game_State {
    bool32 is_initted;
    Render_State render_state;
    Audio_Source music;
};

#endif
