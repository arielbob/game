#ifndef EDITOR_H
#define EDITOR_H

// for some reason putting these defines after the includes breaks emacs indenting
#define LEVEL_FILE_FILTER_TITLE "Levels (*.level)"
#define LEVEL_FILE_FILTER_TYPE  "level"
#define SAVE_SUCCESS_MESSAGE    "Level saved!"

#include "linked_list.h"
#include "level.h"
#include "render.h"

struct Editor_State {
    Heap_Allocator entity_heap;
    Heap_Allocator history_heap;
    Heap_Allocator general_heap;

    Asset_Manager asset_manager;
    
//Linked_List<Entity *> entity_list;
    
//String current_level_name;
    Editor_Level level;

    bool32 use_freecam;
    Camera camera;
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
};

#endif
