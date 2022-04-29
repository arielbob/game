#ifndef GAME_H
#define GAME_H

#define MAX_MESHES 64
#define MAX_MATERIALS 64
#define MAX_ENTITIES 64
#define MAX_POINT_LIGHTS 16
#define MAX_FONTS 64
#define MAX_DEBUG_LINES 64

#include "platform.h"
#include "hash_table.h"
#include "font.h"
#include "ui.h"
#include "mesh.h"
#include "editor.h"
#include "level.h"

#define MAX_PRESSED_CHARS 256

// NOTE: if we change these numbers, we will need to change some allocators since we use the string64 pool
//       often for these strings
#define MESH_NAME_MAX_SIZE       64
#define TEXTURE_NAME_MAX_SIZE    64
#define MATERIAL_NAME_MAX_SIZE   64
#define MATERIAL_STRING_MAX_SIZE 64
#define LEVEL_NAME_MAX_SIZE 64

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

    int32 num_pressed_chars = 0;
    char pressed_chars[MAX_PRESSED_CHARS];

    union {
        Controller_Button_State key_states[17];
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

struct Render_State {
    Display_Output display_output;
    Camera camera;
    Mat4 cpv_matrix;
    Mat4 ortho_clip_matrix;
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

namespace Player_Constants {
    // maximum distance below the player position where the player will instantly walk; if the
    // ground is a larger distance away than this, then the player will fall instead.
    real32 max_lower_ground_offset = 0.2f; 
    // maximum distance above the player position that the player will instantly walk to
    real32 max_upper_ground_offset = 0.5f;
    real32 walk_radius = 0.01f;
    //real32 walk_radius = 1.0f;

    Vec3 forward = make_vec3(0.0f, 0.0f, 1.0f);
    Vec3 right = make_vec3(1.0f, 0.0f, 0.0f);
    Vec3 up = make_vec3(0.0f, 1.0f, 0.0f);

    real32 initial_speed = 1.0f;
};

struct Walk_State {
    Vec3 triangle_normal;
    int32 triangle_index;
    Entity_Type ground_entity_type;
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

    Player player;

    real64 last_update_time;
    real64 last_fps_update_time;
    real64 fps_sum = 0.0f;
    int32 num_fps_samples = 0;
    real32 last_second_fps;

    Render_State render_state;
    Editor_State editor_state;
    Debug_State debug_state;

    Audio_Source music;
    bool32 is_playing_music; // debugging
    UI_Manager ui_manager;
    char current_char; // debugging

    bool32 should_clear_level_gpu_data;
    //String_Buffer level_to_be_loaded;
    Level current_level;

    Hash_Table<int32, Mesh> common_mesh_table;
    Hash_Table<int32, Mesh> primitive_mesh_table;
    Hash_Table<String, File_Data> font_file_table;
    Hash_Table<String, Font> font_table;

    char text_buffer[256] = {}; // debugging
};

namespace Context {
    Game_State *game_state;
    Editor_State *editor_state;
    Controller_State *controller_state;
    UI_Manager *ui_manager;
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
Entity *get_selected_entity(Game_State *game_state);
Font get_font(Game_State *game_state, char *font_name);
//int32 add_material(Game_State *game_state, Material material);
//Texture get_texture(Game_State *game_state, int32 texture_id);
//Mesh get_mesh(Game_State *game_state, int32 mesh_id);
//Mesh *get_mesh_pointer(Game_State *game_state, int32 mesh_id);
//Material get_material(Game_State *game_state, int32 material_id);
//int32 add_mesh(Game_State *game_state, Mesh mesh);
Mesh get_common_mesh(Game_State *game_state, int32 mesh_id);
Mesh get_mesh(Game_State *game_state, Level *level, Mesh_Type mesh_type, int32 mesh_id);
bool32 mesh_exists(Game_State *game_state, Level *level, Mesh_Type mesh_type, int32 mesh_id);
int32 get_mesh_id_by_name(Game_State *game_state, Level *level, Mesh_Type mesh_type, String mesh_name);
Mesh *get_mesh_pointer(Game_State *game_state, Level *level, Mesh_Type mesh_type, int32 mesh_id);
void set_entity_mesh(Game_State *game_state, Level *level, Entity *entity, Mesh_Type mesh_type, int32 mesh_id);
void update_render_state(Render_State *render_state);
Vec3 cursor_pos_to_world_space(Vec2 cursor_pos, Render_State *render_state);
void update_entity_position(Game_State *game_state, Entity *entity, Vec3 new_position);
void update_entity_rotation(Game_State *game_state, Entity *entity, Quaternion new_rotation);
int32 ray_intersects_mesh(Ray ray, Mesh mesh, Transform transform, bool32 include_backside,
                          Ray_Intersects_Mesh_Result *result);
bool32 closest_point_below_on_mesh(Vec3 point, Mesh mesh, Transform transform, Vec3 *result);
bool32 get_walkable_triangle_on_mesh(Vec3 center, real32 radius,
                                     Mesh *mesh, Transform transform,
                                     real32 min_y, real32 max_y,
                                     Get_Walkable_Triangle_On_Mesh_Result *result);
void add_debug_line(Debug_State *debug_state, Vec3 start, Vec3 end, Vec4 color);

#endif
