#ifndef EDITOR_H
#define EDITOR_H

#include "linked_list.h"
#include "level.h"
#include "render.h"
#include "gizmo.h"

// filter_types are kind of platform-specific to the windows open/save file dialogs right now

#define LEVEL_FILE_FILTER_TITLE "Levels (*.level)"
#define LEVEL_FILE_FILTER_TYPE  "*.level"

#define MESH_FILE_FILTER_TITLE "Meshes (*.mesh)"
#define MESH_FILE_FILTER_TYPE  "*.mesh"

#define TEXTURE_FILE_FILTER_TITLE "Image Files (*.png; *.jpg; *.jpeg)"
#define TEXTURE_FILE_FILTER_TYPE  "*.png;*.jpg;*.jpeg"

#define ANIMATION_FILE_FILTER_TITLE "Animations (*.animation)"
#define ANIMATION_FILE_FILTER_TYPE  "*.animation"

#define SAVE_SUCCESS_MESSAGE      "Level saved!"
#define ENTITY_DUPLICATED_MESSAGE "Entity duplicated!"

#define MATERIAL_LIBRARY_WINDOW 1 << 1
#define TEXTURE_LIBRARY_WINDOW  1 << 2
#define MESH_LIBRARY_WINDOW     1 << 3

#define MAX_COLLISION_DEBUG_FRAMES 512

enum class Asset_Library_Tab {
    MATERIALS,
    MESHES,
    TEXTURES,
    ANIMATIONS
};

struct Asset_Library_State {
    bool32 material_modified;
    bool32 material_albedo_color_picker_open;
#if 0
    bool32 material_name_modified;
    bool32 material_albedo_texture_modified;
    
    
    bool32 material_metalness_modified;
    bool32 material_metalness_texture_modified;

    bool32 material_roughness_modified;
    bool32 material_roughness_texture_modified;
    #endif

    int32 selected_material_id = -1;
    int32 selected_mesh_id = -1;
    int32 selected_texture_id = -1;
    int32 selected_animation_id = -1;

    Asset_Library_Tab selected_tab = Asset_Library_Tab::MATERIALS;
};

struct Entity_Properties_State {
    // this is mainly to save heading/pitch/roll so that we don't end up
    // overwriting values with their canonical values. for example, pitch
    // should be allowed to go past 90 degs and below -90 degs, at least
    // when sliding, for a better UX.
    Euler_Transform transform;
    bool32 is_rotation_being_modified;
    bool32 light_color_picker_open;
};

enum Collision_Subframe_Type {
    COLLISION_SUBFRAME_POSITION,
    COLLISION_SUBFRAME_DESIRED_MOVE,
    COLLISION_SUBFRAME_DESIRED_MOVE_COLLISION,
    COLLISION_SUBFRAME_COLLISION
};

// these are attached to the frame
// the frame's player_position is the result of all the intersections that happened that frame
struct Collision_Subframe_Position {
    Vec3 position;
};

struct Collision_Subframe_Desired_Move {
    Vec3 position;
    Vec3 displacement;
    bool32 is_player_move;
};

struct Collision_Subframe_Desired_Move_Collision {
    Vec3 position;
    Vec3 displacement;
    int32 entity_id;
    int32 triangle_index;
    Vec3 triangle_normal;
    Vec3 penetration_normal;
    real32 penetration_depth;
    Vec3 penetration_point;
};

struct Collision_Subframe_Collision {
    Vec3 position;
    int32 entity_id;
    int32 triangle_index;
    Vec3 triangle_normal;
    Vec3 penetration_normal;
    real32 penetration_depth;
    Vec3 penetration_point;
};

struct Collision_Debug_Subframe {
    Collision_Subframe_Type type;
    union {
        Collision_Subframe_Position position;
        Collision_Subframe_Desired_Move desired_move;
        Collision_Subframe_Desired_Move_Collision desired_move_collision;
        Collision_Subframe_Collision collision;
    };
};

#define MAX_COLLISION_DEBUG_FRAME_SUBFRAMES 16

struct Collision_Debug_Frame {
    Collision_Debug_Subframe subframes[MAX_COLLISION_DEBUG_FRAME_SUBFRAMES];
    int32 num_subframes;
};

struct Collision_Debug_State {
    Collision_Debug_Frame debug_frames[MAX_COLLISION_DEBUG_FRAMES];
    int32 debug_frame_start_index;
    int32 num_debug_frames;

    bool32 show_player_capsule = true;
    int32 current_frame;
    int32 current_subframe;
};

struct Editor_State {
    Arena_Allocator *arena;
    
    bool32 is_startup;
    bool32 is_new_level;

    bool32 use_freecam;
    Camera camera;

    bool32 show_wireframe;
    bool32 show_colliders;
    int32 selected_entity_id;
    int32 last_selected_entity_id;

    // this is used to know if anything with the entity changed in the current frame,
    // so we can update UI.
    // it could be a change in the transform, a change in what entity is selected, etc.
    // i.e. anything that should force the entity properties box to update.
    bool32 selected_entity_modified;

    Gizmo_State gizmo_state;

    Asset_Library_State asset_library_state;
    Entity_Properties_State entity_properties_state;
    
    bool32 is_asset_library_window_open;
    bool32 is_collision_debugger_window_open;

    Collision_Debug_State collision_debug_state;
};

namespace Editor_Constants {
    char *editor_font_name = "calibri14";
    char *editor_font_name_bold = "calibri14b";
    char disallowed_chars[] = {'{', '}', '"'};
    int32 num_disallowed_chars = array_length(disallowed_chars);
    real32 point_light_side_length = 0.4f;
    real32 camera_speed = 19.0f;

    Vec4 row_color = make_vec4(0.12f, 0.12f, 0.12f, 0.9f);
    Vec4 darkened_row_color = make_vec4(0.0f, 0.0f, 0.0f, 0.9f);
    Vec4 border_color = make_vec4(0.3f, 0.3f, 0.3f, 1.0f);
    real32 hsv_picker_width = 200.0f;
    real32 hsv_picker_height = 200.0f;
    real32 hue_slider_width = 20.0f;
    real32 medium_padding_x = 10.0f;
    real32 medium_padding_y = 10.0f;
    real32 small_padding_x = 6.0f;
    real32 small_padding_y = 6.0f;
    real32 x_nested_offset = 10.0f;
    real32 color_picker_width = (small_padding_x +
                                 hsv_picker_width + small_padding_x +
                                 hue_slider_width + small_padding_x);
    real32 color_picker_height = 200.0f + small_padding_y*2;
    #if 0
    UI_Color_Picker_Style color_picker_style = { color_picker_width, color_picker_height,
                                                 hsv_picker_width, hsv_picker_height,
                                                 hue_slider_width,
                                                 small_padding_x, small_padding_y,
                                                 row_color };
    #endif

    real32 small_row_height = 20.0f;
    //real32 row_height =  30.0f;
};

#if 0
int32 add_entity(Level *level, Entity *entity);
void delete_entity(Editor_State *editor_state, int32 id);
Entity *get_entity(Editor_State *editor_state, int32 id);
#endif

UI_Button_Theme editor_button_theme = {
    { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE },
    { 0.0f, 20.0f }, { 0.0f, 0.0f },
    DEFAULT_BUTTON_BACKGROUND, DEFAULT_BUTTON_HOT_BACKGROUND, DEFAULT_BUTTON_ACTIVE_BACKGROUND,
    DEFAULT_BUTTON_DISABLED_BACKGROUND,
    { 1.0f, 1.0f, 1.0f, 1.0f },
    default_font
};

UI_Button_Theme editor_button_danger_theme = {
    { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE },
    { 0.0f, 20.0f }, { 0.0f, 0.0f },
    DANGER_BUTTON_BACKGROUND, DANGER_BUTTON_HOT_BACKGROUND, DANGER_BUTTON_ACTIVE_BACKGROUND,
    DEFAULT_BUTTON_DISABLED_BACKGROUND,
    { 1.0f, 1.0f, 1.0f, 1.0f },
    default_font
};

UI_Text_Field_Theme editor_text_field_theme = {
    editor_button_theme.background_color,
    editor_button_theme.hot_background_color,
    editor_button_theme.active_background_color,
    rgb_to_vec4(0, 255, 0),
    0.0f, 0, 0, { 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f,
    default_font,
    { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE },
    { 0.0f, editor_button_theme.size.y }
};

UI_Button_Theme editor_dropdown_item_theme = {
    editor_button_theme.size_type,
    editor_button_theme.size, editor_button_theme.position,
    rgb_to_vec4(19, 19, 23), rgb_to_vec4(36, 36, 43), rgb_to_vec4(8, 8, 10),
    DEFAULT_BUTTON_DISABLED_BACKGROUND,
    editor_button_theme.text_color,
    editor_button_theme.font,
    UI_SCISSOR_INHERIT
};

UI_Button_Theme editor_selected_dropdown_item_theme = {
    editor_button_theme.size_type,
    editor_button_theme.size, editor_button_theme.position,
    rgb_to_vec4(61, 96, 252), rgb_to_vec4(61, 96, 252), rgb_to_vec4(61, 96, 252),
    DEFAULT_BUTTON_DISABLED_BACKGROUND,
    editor_button_theme.text_color,
    editor_button_theme.font,
    UI_SCISSOR_INHERIT
};

UI_Button_Theme item_theme = editor_button_theme;

UI_Button_Theme selected_item_theme = {
    editor_button_theme.size_type,
    editor_button_theme.size, editor_button_theme.position,
    rgb_to_vec4(61, 96, 252),
    rgb_to_vec4(61, 96, 252),
    rgb_to_vec4(61, 96, 252),
    DEFAULT_BUTTON_DISABLED_BACKGROUND,
    editor_button_theme.text_color,
    editor_button_theme.font,
    UI_SCISSOR_INHERIT
};

UI_Text_Field_Slider_Theme editor_slider_theme = {
    DEFAULT_BUTTON_BACKGROUND,
    DEFAULT_BUTTON_HOT_BACKGROUND,
    DEFAULT_BUTTON_ACTIVE_BACKGROUND,
    
    rgb_to_vec4(61, 73, 60),
    rgb_to_vec4(0, 255, 0),
    true,

    0.0f, 0, 0, {}, 0.0f,

    default_font,

    { UI_SIZE_FILL_REMAINING, UI_SIZE_ABSOLUTE },
    { 0.0f, 20.0f }
};

UI_Checkbox_Theme editor_checkbox_theme = {
    DEFAULT_BUTTON_BACKGROUND,
    DEFAULT_BUTTON_HOT_BACKGROUND,
    DEFAULT_BUTTON_ACTIVE_BACKGROUND,

    rgb_to_vec4(255, 255, 255),
    { 20.0f, 20.0f }
};

#endif
