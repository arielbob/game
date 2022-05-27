#ifndef GAME_H
#define GAME_H

#define MAX_MATERIALS 64
#define MAX_TEXTURES 64
#define MAX_ENTITIES 64
#define MAX_POINT_LIGHTS 16
#define MAX_FONTS 64
#define MAX_DEBUG_LINES 64

#include "platform.h"
#include "hash_table.h"
#include "asset.h"
#include "font.h"
#include "ui.h"
#include "mesh.h"
#include "render.h"
#include "editor.h"
//#include "level.h"

#define MAX_PRESSED_CHARS 256

// NOTE: if we change these numbers, we will need to change some allocators since we use the string64 pool
//       often for these strings
#define MESH_NAME_MAX_SIZE       64
#define TEXTURE_NAME_MAX_SIZE    64
#define MATERIAL_NAME_MAX_SIZE   64
#define MATERIAL_STRING_MAX_SIZE 64
#define LEVEL_NAME_MAX_SIZE 64

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

    int32 num_pressed_chars = 0;
    char pressed_chars[MAX_PRESSED_CHARS];

    union {
        Controller_Button_State key_states[19];
        struct {
            Controller_Button_State key_shift;
            Controller_Button_State key_ctrl;
            Controller_Button_State key_alt;
            Controller_Button_State key_tab;
            Controller_Button_State key_w;
            Controller_Button_State key_a;
            Controller_Button_State key_s;
            Controller_Button_State key_d;
            Controller_Button_State key_e;
            Controller_Button_State key_x;
            Controller_Button_State key_z;
            Controller_Button_State key_f5;
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

    // these are in screen-space coordinates in pixels (0,0 at top left)
    Vec2 last_mouse;
    Vec2 current_mouse;
};

enum class Game_Mode {
    EDITING, PLAYING
};

struct Debug_Line {
    Vec3 start;
    Vec3 end;
    Vec4 color;
};

struct Debug_State {
    Debug_Line debug_lines[MAX_DEBUG_LINES];
    int32 num_debug_lines;
};

// first one added is always the one that gets shown first
#define MAX_MESSAGES 8
#define MESSAGE_TIME_LIMIT 1.8f
#define MESSAGE_FADE_START 1.5f
struct Message {
    bool32 is_deallocated;
    real32 timer;
    String text;
};

struct Message_Manager {
    Message messages[MAX_MESSAGES]; // NOTE: should be zero-initialized
    int32 num_messages;
    //int32 first_message_index;
    int32 current_message_index; // index of the next message to be added
    real32 message_time_limit;
    real32 fade_start;
};

namespace Player_Constants {
    // maximum distance below the player position where the player will instantly walk; if the
    // ground is a larger distance away than this, then the player will fall instead.
    real32 max_lower_ground_offset = 0.2f; 
    // maximum distance above the player position that the player will instantly walk to
    real32 max_upper_ground_offset = 0.5f;
    real32 walk_radius = 0.01f;
    //real32 walk_radius = 1.0f;

    real32 capsule_radius = 0.5f;
    real32 player_height = 1.6f;

    Vec3 forward = make_vec3(0.0f, 0.0f, 1.0f);
    Vec3 right = make_vec3(1.0f, 0.0f, 0.0f);
    Vec3 up = make_vec3(0.0f, 1.0f, 0.0f);

    real32 initial_speed = 1.5f;
};

struct Walk_State {
    Vec3 triangle_normal;
    int32 triangle_index;
    int32 ground_entity_id;
};

struct Player {
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;

    real32 heading;
    real32 pitch;
    real32 roll;

    real32 height;
    real32 speed = Player_Constants::initial_speed;

    bool32 is_grounded;
    Walk_State walk_state;
};

struct Game_State {
    bool32 is_initted;
    Game_Mode mode;

    Arena_Allocator level_arena;
    Game_Level level;
    Player player;
    Camera camera;

    real64 last_update_time;
    real64 last_fps_update_time;
    real64 fps_sum = 0.0f;
    int32 num_fps_samples = 0;
    real32 last_second_fps;

    Render_State render_state;
    Editor_State editor_state;
    Debug_State debug_state;

    Message_Manager message_manager;

    Audio_Source music;
    bool32 is_playing_music; // debugging
    UI_Manager ui_manager;
    char current_char; // debugging

    bool32 should_clear_gpu;
    Asset_Manager asset_manager;

    char text_buffer[256] = {}; // debugging
};

namespace Context {
    Game_State *game_state;
    Editor_State *editor_state;
    Controller_State *controller_state;
    UI_Manager *ui_manager;
    Message_Manager *message_manager;
};

struct Ray_Intersects_Mesh_Result {
    real32 t;
    int32 triangle_index;
    Vec3 triangle_normal;
};

struct Get_Walkable_Triangle_On_Mesh_Result {
    Vec3 point;
    int32 triangle_index;
    Vec3 triangle_normal;
};

struct Closest_Vertical_Point_On_Mesh_Result {
    Vec3 point;
    real32 distance_to_point;
    int32 triangle_index;
    Vec3 triangle_normal;
};

inline bool32 was_clicked(Controller_Button_State button_state);
inline bool32 being_held(Controller_Button_State button_state);
inline bool32 just_pressed(Controller_Button_State button_state);
inline bool32 just_lifted(Controller_Button_State button_state);
void update_render_state(Render_State *render_state);
Vec3 cursor_pos_to_world_space(Vec2 cursor_pos, Render_State *render_state);
int32 ray_intersects_mesh(Ray ray, Mesh mesh, Transform transform, bool32 include_backside,
                          Ray_Intersects_Mesh_Result *result);
bool32 capsule_intersects_mesh(Capsule capsule, Mesh mesh, Transform transform,
                               Vec3 *penetration_normal, real32 *penetration_depth);
bool32 closest_point_below_on_mesh(Vec3 point, Mesh mesh, Transform transform, Vec3 *result);
bool32 get_walkable_triangle_on_mesh(Vec3 center, real32 radius,
                                     Mesh *mesh, Transform transform,
                                     real32 min_y, real32 max_y,
                                     Get_Walkable_Triangle_On_Mesh_Result *result);
void add_debug_line(Debug_State *debug_state, Vec3 start, Vec3 end, Vec4 color);
void add_message(Message_Manager *manager, String text);
//void set_entity_transform(Asset_Manager *asset_manager, Entity *entity, Transform transform);

#endif
