#include "editor.h"
#include "level.h"

void draw_row(real32 x, real32 y,
              real32 row_width, real32 row_height,
              Vec4 color, uint32 side_flags,
              bool32 inside_border,
              char *row_id, int32 index) {
    using namespace Context;

    UI_Box_Style box_style = { color };

    do_box(x, y, row_width, row_height,
           box_style, row_id, index);

    // we use boxes instead of lines here because our GL code for drawing lines is really finnicky when
    // trying to draw pixel-perfect horizontal or vertical lines
    Vec4 line_color = make_vec4(0.3f, 0.3f, 0.3f, 1.0f);
    UI_Box_Style line_box_style = { line_color };

    // we draw lines on the inside of the box, so we don't change the expected dimensions of the row
    real32 line_thickness = 1.0f;
    char *border_id = "row_border";
    if (side_flags & SIDE_LEFT) {
        real32 box_x = x;
        if (!inside_border) box_x -= 1;
        do_box(box_x, y, line_thickness, row_height,
               line_box_style, border_id);
    }
    if (side_flags & SIDE_BOTTOM) {
        real32 box_x = x;
        real32 box_y = y + row_height - 1;
        real32 box_width = row_width;
        if (!inside_border) {
            box_y += 1;
            if (side_flags & SIDE_LEFT) {
                box_x -= 1;
                box_width += 1;
            }
            if (side_flags & SIDE_RIGHT) {
                box_width += 1;
            }
        } 
        do_box(box_x, box_y, box_width, line_thickness,
               line_box_style, border_id);
    }
    if (side_flags & SIDE_TOP) {
        real32 box_x = x;
        real32 box_y = y;
        real32 box_width = row_width;
        if (!inside_border) {
            box_y -= 1;
            if (side_flags & SIDE_LEFT) {
                box_x -= 1;
                box_width += 1;
            }
            if (side_flags & SIDE_RIGHT) {
                box_width += 1;
            }
        } 
        do_box(box_x, box_y, box_width, line_thickness,
               line_box_style, border_id);
    }
    if (side_flags & SIDE_RIGHT) {
        real32 box_x = x + row_width - 1;
        if (!inside_border) box_x += 1;
        do_box(box_x, y, line_thickness, row_height,
               line_box_style, border_id);
    }
}

inline void draw_row(real32 x, real32 y,
                     real32 row_width, real32 row_height,
                     Vec4 color, uint32 side_flags,
                     char *row_id, int32 index) {
    return draw_row(x, y,
                    row_width, row_height,
                    color, side_flags,
                    false,
                    row_id, index);
}

inline void draw_row_padding(real32 x, real32 *y,
                             real32 row_width,
                             real32 padding,
                             Vec4 color, uint32 side_flags,
                             char *row_id, int32 index) {
    draw_row(x, *y,
             row_width, padding,
             color, side_flags,
             row_id, index);
    *y += padding;
    // NOTE: we're assuming we're drawing the ui from top to bottom and that if there's a border above a
    //       padding row, there won't be another row above it. if there were a row above this type of
    //       padding row, the border would overlap that box, since we don't change y if there's a top
    //       border.
    if (side_flags & SIDE_BOTTOM) {
        // because we're drawing the border on the outside, we need to offset by the border thickness (1)
        *y += 1;
    }
}

void draw_level_box(UI_Manager *ui_manager, Editor_State *editor_state,
                    Controller_State *controller_state,
                    real32 x, real32 y) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;

    int32 row_index = 0;
    char *row_id = "level_box_row";
    real32 row_width = 198.0f;
    Vec4 row_color = make_vec4(0.1f, 0.1f, 0.1f, 0.9f);

    real32 row_height = 22.0f;
    uint32 side_flags = SIDE_LEFT | SIDE_RIGHT;
    real32 padding_x = 6.0f;
    real32 padding_y = 6.0f;
    real32 initial_x = x;

    real32 button_height = 25.0f;

    draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags | SIDE_TOP, row_id, row_index);

    draw_row(x, y,
             row_width, button_height, row_color, side_flags, row_id, row_index++);

    char *editor_font_name = Editor_Constants::editor_font_name;
    Font font = get_font(asset_manager, editor_font_name);
    
    x += padding_x;
    real32 new_level_button_width = 50.0f;
    bool32 new_level_clicked = do_text_button(x, y,
                                              new_level_button_width, button_height,
                                              default_text_button_style, default_text_style,
                                              "New",
                                              editor_font_name, "new_level");
    if (new_level_clicked) {
        // TODO: do this
    }

    x += new_level_button_width + 1;

    real32 open_level_button_width = 60.0f;
    bool32 open_level_clicked = do_text_button(x, y, 
                                               open_level_button_width, button_height,
                                               default_text_button_style, default_text_style,
                                               "Open",
                                               editor_font_name, "open_level");
    bool32 just_loaded_level = false;
    if (open_level_clicked) {
        Marker m = begin_region();
        char *absolute_filename = (char *) region_push(PLATFORM_MAX_PATH);
        
        if (platform_open_file_dialog(absolute_filename,
                                      LEVEL_FILE_FILTER_TITLE, LEVEL_FILE_FILTER_TYPE,
                                      PLATFORM_MAX_PATH)) {
            Level_Info level_info;
            init_level_info(temp_region, &level_info);
            
            File_Data level_file = platform_open_and_read_file(temp_region, absolute_filename);
            bool32 result = Level_Loader::parse_level_info(temp_region, level_file, &level_info);

            if (result) {
                unload_level(editor_state);
                load_level(editor_state, &level_info);
            }
            
#if 0
            bool32 result = read_and_load_level(asset_manager,
                                                &game_state->current_level, absolute_filename,
                                                &memory.level_arena,
                                                &memory.level_mesh_heap,
                                                &memory.level_string64_pool,
                                                &memory.level_filename_pool);
            if (result) {
                editor_state->is_new_level = false;
                copy_string(&editor_state->current_level_filename, make_string(absolute_filename));
                editor_state->selected_entity_id = -1;
                just_loaded_level = true;
                history_reset(&editor_state->history);
            }
#endif
        }

        end_region(m);
    }

    y += button_height;
    x = initial_x;
    draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags, row_id, row_index++);

    // level name text
    real32 label_row_height = 18.0f;
    draw_row(x, y, row_width, label_row_height, row_color, side_flags, row_id, row_index++);
    do_text(ui_manager, x + padding_x, y + get_center_baseline_offset(label_row_height, get_adjusted_font_height(font)),
            "Level Name", editor_font_name, default_text_style, "level_name_text_box_label");
    y += label_row_height;

    // level name text box
    draw_row(x, y, row_width, row_height, row_color, side_flags, row_id, row_index++);
    UI_Text_Box_Result level_name_result = do_text_box(x + padding_x, y,
                                                       row_width - padding_x*2, row_height,
                                                       make_string(""), 64,
                                                       editor_font_name,
                                                       default_text_box_style, default_text_style,
                                                       just_loaded_level,
                                                       "level_name_text_box");
    String new_level_name = make_string(level_name_result.buffer);
    if (level_name_result.submitted) {
        if (is_empty(new_level_name)) {
            add_message(Context::message_manager, make_string("Level name cannot be empty!"));
        } else if (string_contains(new_level_name,
                                   Editor_Constants::disallowed_chars,
                                   Editor_Constants::num_disallowed_chars)) {
            add_message(Context::message_manager, make_string("Level name cannot contain {, }, or double quotes!"));
        } else {
            if (editor_state->level.name.allocator) {
                replace_with_copy((Allocator *) &editor_state->general_heap,
                                  &editor_state->level.name, new_level_name);
            } else {
                editor_state->level.name = copy((Allocator *) &editor_state->general_heap, new_level_name);
            }
        }
    }
    
    y += row_height;

    // TODO: do this
#if 0
    bool32 level_name_is_valid = (!is_empty(game_state->current_level.name) &&
                                  string_equals(make_string(game_state->current_level.name), new_level_name));

    draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags, row_id, row_index);

    // save level button
    draw_row(x, y, row_width, button_height, row_color, side_flags, row_id, row_index++);

    real32 save_button_width = 50.0f;
    bool32 save_level_clicked = do_text_button(x + padding_x, y,
                                               save_button_width, button_height,
                                               default_text_button_style, default_text_style,
                                               "Save",
                                               editor_font_name,
                                               !level_name_is_valid,
                                               "save_level");

    if (save_level_clicked) {
        assert(!is_empty(game_state->current_level.name));

        Marker m = begin_region();
        char *filename = (char *) region_push(PLATFORM_MAX_PATH);

        if (editor_state->is_new_level) {
            bool32 has_filename = platform_open_save_file_dialog(filename,
                                                                 LEVEL_FILE_FILTER_TITLE, LEVEL_FILE_FILTER_TYPE,
                                                                 PLATFORM_MAX_PATH);

            if (has_filename) {
                export_level((Allocator *) &memory.global_stack, asset_manager, &game_state->current_level, filename);
                copy_string(&editor_state->current_level_filename, make_string(filename));
                editor_state->is_new_level = false;
                
                add_message(&game_state->message_manager, make_string(SAVE_SUCCESS_MESSAGE));
            }
        } else {
            char *level_filename = to_char_array((Allocator *) &memory.global_stack,
                                                 editor_state->current_level_filename);
            export_level((Allocator *) &memory.global_stack, asset_manager, &game_state->current_level, level_filename);
            add_message(&game_state->message_manager, make_string(SAVE_SUCCESS_MESSAGE));
        }
        
        end_region(m);
    }

    x += save_button_width + 1;
    real32 save_as_button_width = 110.0f;
    bool32 save_as_level_clicked = do_text_button(x + padding_x, y,
                                                  save_as_button_width, button_height,
                                                  default_text_button_style, default_text_style,
                                                  "Save As...",
                                                  editor_font_name,
                                                  !level_name_is_valid,
                                                  "save_as_level");

    if (save_as_level_clicked) {
        assert(!is_empty(game_state->current_level.name));

        Marker m = begin_region();
        char *filename = (char *) region_push(PLATFORM_MAX_PATH);

        bool32 has_filename = platform_open_save_file_dialog(filename,
                                                             LEVEL_FILE_FILTER_TITLE, LEVEL_FILE_FILTER_TYPE,
                                                             PLATFORM_MAX_PATH);

        if (has_filename) {
            export_level((Allocator *) &memory.global_stack, asset_manager, &game_state->current_level, filename);
            copy_string(&editor_state->current_level_filename, make_string(filename));
            editor_state->is_new_level = false;
            add_message(&game_state->message_manager, make_string(SAVE_SUCCESS_MESSAGE));
        }
        
        end_region(m);
    }
    x = initial_x;

    y += button_height;
    
#endif
    draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags | SIDE_BOTTOM, row_id, row_index);
}
