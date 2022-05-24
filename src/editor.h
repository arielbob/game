#ifndef EDITOR_H
#define EDITOR_H

#include "editor_actions.h"

// for some reason putting these defines after the includes breaks emacs indenting
#define LEVEL_FILE_FILTER_TITLE "Levels (*.level)"
#define LEVEL_FILE_FILTER_TYPE  "level"
#define SAVE_SUCCESS_MESSAGE    "Level saved!"

#include "linked_list.h"
#include "level.h"
#include "render.h"
#include "gizmo.h"

#define MATERIAL_LIBRARY_WINDOW 1 << 1
#define TEXTURE_LIBRARY_WINDOW  1 << 2
#define MESH_LIBRARY_WINDOW     1 << 3

struct Editor_State {
    Heap_Allocator entity_heap;
    Heap_Allocator history_heap;
    Heap_Allocator general_heap;

    Asset_Manager asset_manager;

    Editor_History history;

    Editor_Level level;
    String level_filename;
    bool32 is_startup;
    bool32 is_new_level;
    bool32 should_unload_level_gpu_data;

    bool32 use_freecam;
    Camera camera;

    bool32 show_wireframe;
    bool32 show_colliders;
    int32 selected_entity_id;
    int32 last_selected_entity_id;

    uint32 open_window_flags;
    UI_id color_picker_parent;
    
    Gizmo_State gizmo_state;

    // TODO: maybe use flags for these instead
    bool32 editing_selected_entity_mesh;
    bool32 editing_selected_entity_material;
    bool32 editing_selected_entity_texture;
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
    real32 small_padding_x = 6.0f;
    real32 small_padding_y = 6.0f;
    real32 x_nested_offset = 10.0f;
    real32 color_picker_width = (small_padding_x +
                                 hsv_picker_width + small_padding_x +
                                 hue_slider_width + small_padding_x);
    real32 color_picker_height = 200.0f + small_padding_y*2;
    UI_Color_Picker_Style color_picker_style = { color_picker_width, color_picker_height,
                                                 hsv_picker_width, hsv_picker_height,
                                                 hue_slider_width,
                                                 small_padding_x, small_padding_y,
                                                 row_color };

    real32 small_row_height = 20.0f;
    //real32 row_height =  30.0f;
};

int32 add_entity(Editor_Level *level, Entity *entity, int32 existing_id = -1);
void delete_entity(Editor_State *editor_state, int32 id);
Entity *get_entity(Editor_State *editor_state, int32 id);
void unload_level(Editor_State *editor_state);
void load_level(Editor_State *editor_state, Level_Info *level_info);
bool32 read_and_load_level(Editor_State *editor_state, char *filename);
void reset_editor(Editor_State *editor_state);

#endif
