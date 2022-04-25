#include "font.h"
#include "editor.h"
#include "game.h"

#define ROW_HEIGHT 30.0f
#define SMALL_ROW_HEIGHT 20.0f

#if 0
void set_temp_material(Editor_State *editor_state, Material material) {
    Material *temp_material = &editor_state->temp_material;
    copy_string(&temp_material->name, &material.name);
    copy_string(&temp_material->texture_name, &material.texture_name);

    temp_material->gloss = material.gloss;
    temp_material->color_override = material.color_override;
    temp_material->use_color_override = material.use_color_override;
}
#endif

#if 0
char *editor_font_name = "courier16";
char *editor_font_name_bold = "courier16b";
#else
char *editor_font_name = "calibri14";
#endif

// for comparing current to new
bool32 selected_entity_changed(Editor_State *editor_state,
                               int32 new_entity_index, Entity_Type new_entity_type) {
    return ((new_entity_index != editor_state->selected_entity_index) ||
            (new_entity_type != editor_state->selected_entity_type));
}

// for comparing last to current
bool32 selected_entity_changed(Editor_State *editor_state) {
    return ((editor_state->selected_entity_index != editor_state->last_selected_entity_index) ||
            (editor_state->selected_entity_type != editor_state->last_selected_entity_type));
}

void reset_entity_editors(Editor_State *editor_state) {
    editor_state->open_window_flags = 0;
    editor_state->color_picker.parent_ui_id = {};
}

bool32 editor_add_mesh_press(Allocator *string_allocator, Allocator *filename_allocator,
                             Level *level, Entity *entity) {
    Marker m = begin_region();
    char *absolute_filename = (char *) region_push(PLATFORM_MAX_PATH);
        
    if (platform_open_file_dialog(absolute_filename, PLATFORM_MAX_PATH)) {
        String_Buffer new_mesh_name_buffer = make_string_buffer(string_allocator, 64);
            
        char *relative_filename = (char *) region_push(PLATFORM_MAX_PATH);
        platform_get_relative_path(absolute_filename, relative_filename, PLATFORM_MAX_PATH);
        String_Buffer new_mesh_filename_buffer = make_string_buffer(filename_allocator,
                                                                    relative_filename, PLATFORM_MAX_PATH);

        Mesh new_mesh = read_and_load_mesh((Allocator *) &memory.mesh_arena,
                                           new_mesh_filename_buffer,
                                           new_mesh_name_buffer);
        int32 mesh_id = level_add_mesh(level, new_mesh);

        set_entity_mesh(Context::game_state, level, entity, Mesh_Type::LEVEL, mesh_id);
        end_region(m);
        return true;
    }

    // TODO: error handling (after in-game console implementation)

    end_region(m);
    return false;
}

void draw_row_line(real32 x, real32 *y,
                   real32 row_width, bool32 draw_outside_row = true) {
    Vec4 line_color = make_vec4(0.3f, 0.3f, 0.3f, 1.0f);
    UI_Box_Style line_box_style = { line_color };
    real32 line_thickness = 1.0f;
    char *box_id = "row_line";
    if (draw_outside_row) {
        x -= 1.0f;
        row_width += 2.0f;
    }
    do_box(x, *y, row_width, line_thickness,
           line_box_style, box_id);
    *y += 1;
}

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
                             int32 index) {
    draw_row(x, *y,
             row_width, padding,
             color, side_flags,
             "row_padding", index);
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

void draw_centered_text(real32 box_x, real32 box_y,
                        real32 row_width, real32 row_height,
                        char *text, char *font_name, UI_Text_Style text_style) {
    using namespace Context;
    Font font = get_font(game_state, font_name);
    real32 adjusted_text_height = font.height_pixels - font.scale_for_pixel_height * (font.ascent + font.descent);
    real32 x_offset = 0.5f * row_width - 0.5f * get_width(font, text);
    real32 y_offset = 0.5f * (row_height + adjusted_text_height);
    do_text(ui_manager, box_x + x_offset, box_y + y_offset,
            text, font_name, text_style, "entity_properties_text");
}

void draw_v_centered_text(real32 box_x, real32 box_y,
                          real32 row_height,
                          char *text, char *font_name, UI_Text_Style text_style) {
    using namespace Context;
    Font font = get_font(game_state, font_name);
    real32 adjusted_text_height = font.height_pixels - font.scale_for_pixel_height * (font.ascent + font.descent);
    real32 y_offset = 0.5f * (row_height + adjusted_text_height);
    do_text(ui_manager, box_x, box_y + y_offset,
            text, font_name, text_style, "entity_properties_text");
}

void draw_labeled_text(real32 x, real32 y,
                       real32 row_height,
                       char *label_font, char *label,
                       char *text_font, char *text,
                       UI_Text_Style text_style) {
    using namespace Context;
    real32 small_spacing = 20.0f;
    draw_v_centered_text(x, y, row_height,
                         label, label_font, text_style);
    draw_v_centered_text(x+small_spacing, y, row_height,
                         text, text_font, text_style);
}

void draw_mesh_library(Game_State *game_state, Controller_State *controller_state, Entity *selected_entity) {
    Render_State *render_state = &game_state->render_state;
    UI_Manager *ui_manager = &game_state->ui_manager;
    Editor_State *editor_state = &game_state->editor_state;

    push_layer(ui_manager);

    real32 padding_x = 20.0f;
    real32 padding_y = 20.0f;

    real32 x_gap = 10.0f;
    real32 y_gap = 10.0f;

    int32 num_items_per_row = 5;
    real32 item_width = 100.0f;
    real32 item_height = 120.0f;

    real32 window_width = padding_x * 2 + x_gap * (num_items_per_row - 1) + num_items_per_row*item_width;

    real32 title_row_height = 50.0f;
    Vec4 title_row_color = make_vec4(0.05f, 0.2f, 0.5f, 1.0f);
    Vec4 row_color = make_vec4(0.1f, 0.1f, 0.1f, 1.0f);

    Font font = get_font(game_state, editor_font_name);
    real32 button_padding_x = 15.0f;

    char *row_id = "mesh_library_row";
    int32 row_index = 0;

    real32 initial_x = render_state->display_output.width / 2.0f - window_width / 2.0f;
    real32 x = initial_x;
    real32 y = 80.0f;

    UI_Text_Style text_style = default_text_style;

    draw_row(x, y, window_width, title_row_height, title_row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_TOP | SIDE_BOTTOM, row_id, row_index++);
    draw_centered_text(x, y, window_width, title_row_height,
                       "Mesh Library", editor_font_name, text_style);
    y += title_row_height;

    real32 content_height = 500.0f;
    draw_row(x, y, window_width, content_height, row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_BOTTOM, row_id, row_index++);    

    real32 cancel_button_width = 100.0f;
    bool32 cancel_pressed = do_text_button(x + padding_x,
                                           y + content_height - SMALL_ROW_HEIGHT - padding_y - 1,
                                           cancel_button_width, SMALL_ROW_HEIGHT,
                                           default_text_button_cancel_style, default_text_style,
                                           "Cancel",
                                           editor_font_name,
                                           "choose_mesh_cancel");
    if (cancel_pressed) {
        editor_state->open_window_flags = 0;
    }

    Allocator *allocator = (Allocator *) &memory.frame_arena;

    x += padding_x;
    y += padding_y;

    UI_Text_Button_Style filter_button_style = default_text_button_style;
    UI_Text_Button_Style level_item_style, primitive_item_style;

    real32 filter_button_width = get_width(font, "All") + button_padding_x*2;
    real32 filter_button_height = SMALL_ROW_HEIGHT;

    real32 button_gap = 5.0f;
    bool32 all_filter_pressed = do_text_button(x, y,
                                               filter_button_width, filter_button_height,
                                               filter_button_style, default_text_style,
                                               "All",
                                               editor_font_name,
                                               editor_state->mesh_library_filter == Mesh_Type::NONE,
                                               "mesh_filter_all");
    if (all_filter_pressed) editor_state->mesh_library_filter = Mesh_Type::NONE;
    x += filter_button_width + button_gap;

    filter_button_width = get_width(font, "Level") + button_padding_x*2;
    filter_button_style.normal_color = rgb_to_vec4(35, 148, 39);
    filter_button_style.hot_color = rgb_to_vec4(42, 184, 78);
    filter_button_style.active_color = rgb_to_vec4(14, 87, 33);
    level_item_style = filter_button_style;

    bool32 level_filter_pressed = do_text_button(x, y,
                                                 filter_button_width, filter_button_height,
                                                 filter_button_style, default_text_style,
                                                 "Level",
                                                 editor_font_name,
                                                 editor_state->mesh_library_filter == Mesh_Type::LEVEL,
                                                 "mesh_filter_level");
    if (level_filter_pressed) editor_state->mesh_library_filter = Mesh_Type::LEVEL;
    x += filter_button_width + button_gap;

    char *primitives_filter_text = "Primitives";
    filter_button_width = get_width(font, primitives_filter_text) + button_padding_x*2;
    filter_button_style.normal_color = rgb_to_vec4(150, 41, 125);
    filter_button_style.hot_color = rgb_to_vec4(212, 57, 155);
    filter_button_style.active_color = rgb_to_vec4(71, 14, 50);
    primitive_item_style = filter_button_style;

    bool32 primitives_pressed = do_text_button(x, y,
                                               filter_button_width, filter_button_height,
                                               filter_button_style, default_text_style,
                                               primitives_filter_text,
                                               editor_font_name,
                                               editor_state->mesh_library_filter == Mesh_Type::PRIMITIVE,
                                               "mesh_filter_primitives");
    if (primitives_pressed) editor_state->mesh_library_filter = Mesh_Type::PRIMITIVE;
    x += filter_button_width;

    y += filter_button_height + x_gap;
                   
    x = initial_x + padding_x;
    UI_Image_Button_Style image_button_style = default_image_button_style;
    image_button_style.image_constraint_flags = CONSTRAINT_FILL_BUTTON_WIDTH | CONSTRAINT_KEEP_IMAGE_PROPORTIONS;

    Hash_Table<int32, Mesh> *mesh_table = &game_state->current_level.mesh_table;
    Mesh_Type picked_mesh_type = Mesh_Type::NONE;
    int32 picked_mesh_id = -1;

    if (editor_state->mesh_library_filter == Mesh_Type::NONE ||
        editor_state->mesh_library_filter == Mesh_Type::LEVEL) {
        for (int32 i = 0; i < mesh_table->max_entries; i++) {
            Hash_Table_Entry<int32, Mesh> *entry = &mesh_table->entries[i];
            if (!entry->is_occupied) continue;

            Mesh *mesh = &entry->value;
            char *mesh_name = to_char_array(allocator, mesh->name);
            bool32 pressed = do_text_button(x, y,
                                            item_width, item_height,
                                            level_item_style, default_text_style,
                                            mesh_name,
                                            editor_font_name,
                                            "mesh_library_level_item", i);

            if (pressed) {
                picked_mesh_type = Mesh_Type::LEVEL;
                picked_mesh_id = entry->key;
            }

            x += item_width + x_gap;
            if (x + item_width > initial_x + window_width) {
                x = initial_x + padding_x;
                y += item_height + y_gap;
            }
        }
    }

    if (editor_state->mesh_library_filter == Mesh_Type::NONE ||
        editor_state->mesh_library_filter == Mesh_Type::PRIMITIVE) {
        mesh_table = &game_state->primitive_mesh_table;
        for (int32 i = 0; i < mesh_table->max_entries; i++) {
            Hash_Table_Entry<int32, Mesh> *entry = &mesh_table->entries[i];
            if (!entry->is_occupied) continue;

            Mesh *mesh = &entry->value;
            char *mesh_name = to_char_array(allocator, mesh->name);
            bool32 pressed = do_text_button(x, y,
                                            item_width, item_height,
                                            primitive_item_style, default_text_style,
                                            mesh_name,
                                            editor_font_name,
                                            "mesh_library_primitive_item", i);

            if (pressed) {
                picked_mesh_type = Mesh_Type::PRIMITIVE;
                picked_mesh_id = entry->key;
            }

            x += item_width + x_gap;
            if (x + item_width > initial_x + window_width) {
                x = initial_x + padding_x;
                y += item_height + y_gap;
            }
        }
    }
    
    if (picked_mesh_id >= 0) {
        if (selected_entity->mesh_id < 0 ||
            picked_mesh_id != selected_entity->mesh_id ||
            picked_mesh_type != selected_entity->mesh_type) {
            set_entity_mesh(game_state, &game_state->current_level, selected_entity,
                            picked_mesh_type, picked_mesh_id);
        }

        editor_state->open_window_flags = 0;
    }

    pop_layer(ui_manager);
}

void draw_texture_library(Material *selected_material) {
    using namespace Context;
    Render_State *render_state = &game_state->render_state;

    push_layer(ui_manager);

    real32 padding_x = 20.0f;
    real32 padding_y = 20.0f;

    real32 x_gap = 10.0f;
    real32 y_gap = 10.0f;

    int32 num_items_per_row = 5;
    real32 item_width = 100.0f;
    real32 item_height = 120.0f;

    real32 window_width = padding_x * 2 + x_gap * (num_items_per_row - 1) + num_items_per_row*item_width;

    real32 title_row_height = 50.0f;
    Vec4 title_row_color = make_vec4(0.05f, 0.2f, 0.5f, 1.0f);
    Vec4 row_color = make_vec4(0.1f, 0.1f, 0.1f, 1.0f);

    char *row_id = "texture_library_row";
    int32 row_index = 0;

    real32 initial_x = render_state->display_output.width / 2.0f - window_width / 2.0f;
    real32 x = initial_x;
    real32 y = 80.0f;

    UI_Text_Style text_style = default_text_style;

    draw_row(x, y, window_width, title_row_height, title_row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_TOP | SIDE_BOTTOM, row_id, row_index++);
    draw_centered_text(x, y, window_width, title_row_height,
                       "Texture Library", editor_font_name, text_style);
    y += title_row_height;

    real32 content_height = 500.0f;
    draw_row(x, y, window_width, content_height, row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_BOTTOM, row_id, row_index++);    

    real32 cancel_button_width = 100.0f;
    bool32 cancel_pressed = do_text_button(x + padding_x,
                                           y + content_height - SMALL_ROW_HEIGHT - padding_y - 1,
                                           cancel_button_width, SMALL_ROW_HEIGHT,
                                           default_text_button_cancel_style, default_text_style,
                                           "Cancel",
                                           editor_font_name,
                                           "choose_texture_cancel");
    if (cancel_pressed) {
        editor_state->open_window_flags = 0;
    }

    Allocator *allocator = (Allocator *) &memory.frame_arena;

    x += padding_x;
    y += padding_y;

    UI_Image_Button_Style image_button_style = default_image_button_style;
    image_button_style.image_constraint_flags = CONSTRAINT_FILL_BUTTON_WIDTH | CONSTRAINT_KEEP_IMAGE_PROPORTIONS;

    Hash_Table<int32, Texture> *texture_table = &game_state->current_level.texture_table;
    char *button_id_string = "texture_library_item";
    int32 picked_texture_id = -1;
    for (int32 i = 0; i < texture_table->max_entries; i++) {
        Hash_Table_Entry<int32, Texture> *entry = &texture_table->entries[i];
        if (!entry->is_occupied) continue;

        Texture *texture = &entry->value;
        char *texture_name = to_char_array(allocator, texture->name);
        bool32 pressed = do_image_button(x, y,
                                         item_width, item_height,
                                         image_button_style, default_text_style,
                                         entry->key, texture_name, editor_font_name,
                                         button_id_string, i);

        if (pressed) picked_texture_id = entry->key;
        x += item_width + x_gap;
        if (x + item_width > initial_x + window_width) {
            x = initial_x + padding_x;
            y += item_height + y_gap;
        }
    }
    
    if (picked_texture_id >= 0) {
        if (selected_material->texture_id < 0 ||
            picked_texture_id != selected_material->texture_id) {
            selected_material->texture_id = picked_texture_id;
        }

        editor_state->open_window_flags = 0;
    }

    pop_layer(ui_manager);
}

void draw_material_library(Game_State *game_state, Controller_State *controller_state,
                           Entity *entity) {
    Render_State *render_state = &game_state->render_state;
    UI_Manager *ui_manager = &game_state->ui_manager;
    Editor_State *editor_state = &game_state->editor_state;

    push_layer(ui_manager);
    
    real32 padding_x = 20.0f;
    real32 padding_y = 20.0f;

    real32 x_gap = 10.0f;
    real32 y_gap = 10.0f;

    int32 num_items_per_row = 1;
    real32 item_width = 500.0f;
    real32 item_height = 20.0f;

    real32 window_width = padding_x * 2 + x_gap * (num_items_per_row - 1) + num_items_per_row*item_width;

    real32 title_row_height = 50.0f;
    Vec4 title_row_color = make_vec4(0.05f, 0.2f, 0.5f, 1.0f);
    Vec4 row_color = make_vec4(0.1f, 0.1f, 0.1f, 1.0f);

    char *row_id = "material_library_row";
    int32 row_index = 0;

    real32 initial_x = render_state->display_output.width / 2.0f - window_width / 2.0f;
    real32 x = initial_x;
    real32 y = 80.0f;

    UI_Text_Style text_style = default_text_style;

    draw_row(x, y, window_width, title_row_height, title_row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_TOP | SIDE_BOTTOM, row_id, row_index++);
    draw_centered_text(x, y, window_width, title_row_height,
                       "Material Library", editor_font_name, text_style);
    y += title_row_height;

    real32 content_height = 500.0f;
    draw_row(x, y, window_width, content_height, row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_BOTTOM, row_id, row_index++);    

    real32 cancel_button_width = 100.0f;
    bool32 cancel_pressed = do_text_button(x + padding_x,
                                           y + content_height - SMALL_ROW_HEIGHT - padding_y - 1,
                                           cancel_button_width, SMALL_ROW_HEIGHT,
                                           default_text_button_cancel_style, default_text_style,
                                           "Cancel",
                                           editor_font_name,
                                           "choose_material_cancel");
    if (cancel_pressed) {
        editor_state->open_window_flags = 0;
    }
    
    Allocator *allocator = (Allocator *) &memory.frame_arena;

    x += padding_x;
    y += padding_y;

    Hash_Table<int32, Material> *material_table = &game_state->current_level.material_table;
    char *button_id_string = "material_library_item";
    int32 picked_material_id = -1;
    for (int32 i = 0; i < material_table->max_entries; i++) {
        Hash_Table_Entry<int32, Material> *entry = &material_table->entries[i];
        if (!entry->is_occupied) continue;

        Material *material = &entry->value;
        bool32 pressed = do_text_button(x, y,
                                        item_width, item_height,
                                        default_text_button_style, default_text_style,
                                        to_char_array(allocator, material->name),
                                        editor_font_name,
                                        button_id_string, i);

        if (pressed) picked_material_id = entry->key;
        x += item_width + x_gap;
        if (x + item_width > initial_x + window_width) {
            x = initial_x + padding_x;
            y += item_height + y_gap;
        }
    }
    
    if (picked_material_id >= 0) {
        if (picked_material_id != entity->material_id) {
            entity->material_id = picked_material_id;
        }

        editor_state->open_window_flags = 0;
    }

    pop_layer(ui_manager);
};

void open_color_picker(Editor_Color_Picker *editor_color_picker, UI_id parent_id, Vec4 initial_color) {
    editor_color_picker->parent_ui_id = parent_id;
    editor_color_picker->ui_state = make_color_picker_state(initial_color,
                                                            Editor_Constants::hsv_picker_width,
                                                            Editor_Constants::hsv_picker_height);
}
                       
void draw_entity_box(Game_State *game_state, Controller_State *controller_state, Entity *entity) {
    int32 row_index = 0;

    UI_Manager *ui_manager = &game_state->ui_manager;
    Editor_State *editor_state = &game_state->editor_state;

    real32 box_x = 5.0f;
    real32 box_y = 50.0f;

    real32 box_padding_x = 10.0f;
    real32 box_padding_y = 10.0f;

    Allocator *allocator = (Allocator *) &memory.frame_arena;

    Level *level = &game_state->current_level;
    Mesh *mesh = get_mesh_pointer(game_state, level, entity->mesh_type, entity->mesh_id);
    // bool32 material_exists = hash_table_find_pointer(game_state->current_level.material_table,
    //                                                  entity->material_id,
    //                                                  &material);
    // assert(material_exists);
    Transform transform = entity->transform;

    UI_Text_Button_Style button_style = default_text_button_style;
    
    UI_Text_Style text_style = default_text_style;
    
    real32 padding_left = 6.0f;
    real32 padding_right = padding_left;
    real32 padding_top = padding_left;
    real32 padding_bottom = padding_left;
    real32 padding_y = padding_top;
    real32 right_column_offset = padding_left + 120.0f;
    real32 small_spacing = 20.0f;

    real32 initial_row_height = 22.0f;
    real32 row_height = initial_row_height;
    real32 small_row_height = SMALL_ROW_HEIGHT;
    real32 row_width = 400.0f;

    Vec4 title_row_color = make_vec4(0.05f, 0.2f, 0.5f, 1.0f);
    Vec4 row_color = make_vec4(0.1f, 0.1f, 0.1f, 0.9f);

    real32 initial_x = box_x;
    real32 x = box_x;
    real32 y = box_y;

    real32 title_row_height = 30.0f;
    uint32 side_flags = SIDE_LEFT | SIDE_RIGHT;

    char *row_id = "mesh_properties_line";

    draw_row(x, y, row_width, title_row_height, title_row_color,
             side_flags | SIDE_TOP, row_id, row_index++);
    draw_centered_text(x, y, row_width, title_row_height,
                       "Entity Properties", editor_font_name, text_style);
    y += title_row_height;
    draw_row_line(x, &y, row_width);
        
    real32 choose_mesh_button_width = 200.0f;
    real32 add_mesh_button_width = 30.0f;
    choose_mesh_button_width -= add_mesh_button_width;

    char *mesh_name = to_char_array(allocator, mesh->name);
    draw_row_padding(x, &y, row_width, padding_top, row_color, side_flags,
                     row_index++);
    draw_row(x, y, row_width, row_height, row_color, side_flags,
             row_id, row_index++);
    char *mesh_label_string = "Mesh";
    if (entity->mesh_type == Mesh_Type::PRIMITIVE) mesh_label_string = "Mesh (primitive)";
    draw_v_centered_text(x + padding_left, y, row_height,
                         mesh_label_string, editor_font_name, text_style);

    x += right_column_offset;
    bool32 choose_mesh_pressed = do_text_button(x, y,
                                                choose_mesh_button_width, row_height,
                                                button_style, default_text_style,
                                                to_char_array(allocator, mesh->name),
                                                editor_font_name, "choose_mesh");

    if (!editor_state->open_window_flags && choose_mesh_pressed) {
        editor_state->open_window_flags |= MESH_LIBRARY_WINDOW;
    }
    x += choose_mesh_button_width + padding_left;

    bool32 add_mesh_pressed = do_text_button(x, y,
                                             add_mesh_button_width, row_height,
                                             button_style, default_text_style,
                                             "+", editor_font_name, "add_mesh");
    x += add_mesh_button_width + padding_left;

    if (add_mesh_pressed) {
        Allocator *string64_allocator = (Allocator *) &memory.string64_pool;
        Allocator *filename_allocator = (Allocator *) &memory.filename_pool;
        bool32 mesh_added = editor_add_mesh_press(string64_allocator, filename_allocator,
                                                  &game_state->current_level, entity);
        if (mesh_added) {
            editor_state->editing_selected_entity_mesh = true;
        }
    }

    real32 edit_mesh_button_width = row_width - (x - initial_x) - padding_right;
    bool32 edit_mesh_pressed = do_text_button(x, y,
                                              edit_mesh_button_width, row_height,
                                              button_style, default_text_style,
                                              "Edit", editor_font_name,
                                              entity->mesh_type == Mesh_Type::PRIMITIVE,
                                              "edit_mesh");

    if (edit_mesh_pressed) {
        editor_state->editing_selected_entity_mesh = !editor_state->editing_selected_entity_mesh;
    }

    y += row_height;
    x = initial_x;
    if (editor_state->editing_selected_entity_mesh) {
        draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags,
            row_index++);
        UI_Text_Box_Style text_box_style = default_text_box_style;
        real32 edit_box_width = choose_mesh_button_width + padding_left + add_mesh_button_width;

        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(x + padding_left, y, row_height,
                             "Mesh Name", editor_font_name, default_text_style);
        do_text_box(x + right_column_offset, y,
                    edit_box_width, row_height,
                    &mesh->name, editor_font_name,
                    text_box_style, default_text_style,
                    "mesh_name_text_box");
        y += row_height;
        
    }
    draw_row_padding(x, &y, row_width, padding_y, row_color,
                     side_flags | SIDE_BOTTOM, row_index++);

    x = initial_x;

    char *buf;
    int32 buffer_size = 16;
    
    // POSITION
    draw_row(x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    draw_v_centered_text(x+padding_left, y, small_row_height,
                         "Position", editor_font_name, text_style);
    // x
    buf = string_format(allocator, buffer_size, "%f", transform.position.x);
    draw_labeled_text(x+right_column_offset, y, small_row_height,
                      editor_font_name, "x", editor_font_name, buf, text_style);
    y += small_row_height;
    // y
    draw_row(x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.position.y);
    draw_labeled_text(x + right_column_offset, y, small_row_height,
                      editor_font_name, "y", editor_font_name, buf, text_style);
    y += small_row_height;
    // z
    draw_row(x, y, row_width, small_row_height, row_color, side_flags,
             row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.position.z);
    draw_labeled_text(x + right_column_offset, y, small_row_height,
                      editor_font_name, "z", editor_font_name, buf, text_style);
    y += small_row_height;    

    draw_row_line(x, &y, row_width);

    // ROTATION
    draw_row(x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    draw_v_centered_text(x+padding_left, y, small_row_height,
                         "Rotation", editor_font_name, text_style);
    // w
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.w);
    draw_labeled_text(x + right_column_offset, y, small_row_height,
                      editor_font_name, "w", editor_font_name, buf, text_style);
    y += small_row_height;
    // x
    draw_row(x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.v.x);
    draw_labeled_text(x + right_column_offset, y, small_row_height,
                      editor_font_name, "x", editor_font_name, buf, text_style);
    y += small_row_height;
    // y
    draw_row(x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.v.y);
    draw_labeled_text(x + right_column_offset, y, small_row_height,
                      editor_font_name, "y", editor_font_name, buf, text_style);
    y += small_row_height;
    // z
    draw_row(x, y, row_width, small_row_height, row_color, side_flags,
             row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.v.z);
    draw_labeled_text(x + right_column_offset, y, small_row_height,
                      editor_font_name, "z", editor_font_name, buf, text_style);
    y += small_row_height;

    draw_row_line(x, &y, row_width);

    // SCALE
    draw_row(x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    draw_v_centered_text(x+padding_left, y, small_row_height,
                         "Scale", editor_font_name, text_style);
    // x
    buf = string_format(allocator, buffer_size, "%f", transform.scale.x);
    draw_labeled_text(x + right_column_offset, y, small_row_height,
                      editor_font_name, "x", editor_font_name, buf, text_style);
    y += small_row_height;
    // y
    draw_row(x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.scale.y);
    draw_labeled_text(x + right_column_offset, y, small_row_height,
                      editor_font_name, "y", editor_font_name, buf, text_style);
    y += small_row_height;
    // z
    draw_row(x, y, row_width, small_row_height, row_color, side_flags | SIDE_BOTTOM,
             row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.scale.z);
    draw_labeled_text(x + right_column_offset, y, small_row_height,
                      editor_font_name, "z", editor_font_name, buf, text_style);
    y += small_row_height;

    draw_row_line(x, &y, row_width);

    // MATERIAL
    draw_row_padding(x, &y, row_width, padding_bottom, row_color,
                     side_flags, row_index++);

    draw_row(x, y, row_width, row_height, row_color, side_flags,
             row_id, row_index++);
    draw_v_centered_text(x+padding_left, y, row_height,
                         "Material", editor_font_name, text_style);

    side_flags = SIDE_LEFT | SIDE_RIGHT;

    real32 choose_material_button_width = 200.0f;
    real32 add_material_button_width = 30.0f;
    choose_material_button_width -= add_material_button_width;

    x += right_column_offset;
    Material *material = NULL;
    char *material_name = "";
    bool32 has_material = entity->material_id >= 0;
    if (has_material) {
        material = get_material_pointer(level, entity->material_id);
        material_name = to_char_array(allocator, material->name);
    }
    bool32 choose_material_pressed = do_text_button(x, y,
                                                    choose_material_button_width, row_height,
                                                    button_style, default_text_style,
                                                    material_name,
                                                    editor_font_name, "choose_material");

    if (!editor_state->open_window_flags && choose_material_pressed) {
        editor_state->open_window_flags |= MATERIAL_LIBRARY_WINDOW;
    }
    x += choose_material_button_width + padding_left;

    bool32 add_material_pressed = do_text_button(x, y,
                                                 add_material_button_width, row_height,
                                                 button_style, default_text_style,
                                                 "+", editor_font_name, "add_material");
    x += add_material_button_width + padding_left;

    if (add_material_pressed) {
        Pool_Allocator *string64_pool = &memory.string64_pool;

        Material new_material = { 
            make_string_buffer((Allocator *) string64_pool, "New Material", MATERIAL_STRING_MAX_SIZE),
            -1,
            50.0f,
            make_vec4(0.0f, 0.0f, 0.0f, 1.0f),
            true
        };

        int32 material_id = level_add_material(level, new_material);
        entity->material_id = material_id;
        editor_state->editing_selected_entity_material = true;
    }

    real32 edit_material_button_width = row_width - (x - initial_x) - padding_right;
    bool32 edit_material_pressed = do_text_button(x, y,
                                                  edit_material_button_width, row_height,
                                                  button_style, default_text_style,
                                                  "Edit", editor_font_name,
                                                  !has_material,
                                                  "edit_material");
    
    x = initial_x;
    y += row_height;

    if (edit_material_pressed) {
        editor_state->editing_selected_entity_material = !editor_state->editing_selected_entity_material;
    }

    real32 edit_box_width = choose_material_button_width + padding_left + add_material_button_width;
    if (has_material && editor_state->editing_selected_entity_material) {
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);

        UI_Text_Box_Style text_box_style = default_text_box_style;

        // MATERIAL NAME
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(x + padding_left, y, row_height,
                             "Material Name", editor_font_name, default_text_style);
        do_text_box(x + right_column_offset, y,
                    edit_box_width, row_height,
                    &material->name, editor_font_name,
                    text_box_style, default_text_style,
                    "material_name_text_box");

        y += row_height;
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);

        // TEXTURE
        char *texture_name = "";
        if (material->texture_id >= 0) {
            Texture texture = get_texture(level, material->texture_id);
            texture_name = to_char_array(allocator, texture.name);
        }
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(x + padding_left, y, row_height,
                             "Texture", editor_font_name, text_style);
        bool32 choose_texture_pressed = do_text_button(x + right_column_offset, y,
                                                       edit_box_width, row_height,
                                                       button_style, default_text_style,
                                                       texture_name, editor_font_name, "choose_texture");
        if (!editor_state->open_window_flags && choose_texture_pressed) {
            editor_state->open_window_flags |= TEXTURE_LIBRARY_WINDOW;
        }

        y += row_height;
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);

        // GLOSS
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        buf = string_format(allocator, buffer_size, "%f", material->gloss);
        draw_v_centered_text(x + padding_left, y, row_height,
                             "Gloss", editor_font_name, text_style);
        material->gloss = do_slider(x+right_column_offset, y,
                                    edit_box_width, row_height,
                                    buf, editor_font_name,
                                    0.0f, 100.0f, material->gloss,
                                    default_slider_style, default_text_style,
                                    "edit_material_gloss_slider");
        y += row_height;
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);

        // COLOR OVERRIDE
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        buf = string_format(allocator, buffer_size, "%f", material->gloss);
        draw_v_centered_text(x + padding_left, y, row_height,
                             "Color Override", editor_font_name, text_style);
        UI_id color_override_button_id;
        bool32 color_override_pressed = do_color_button(x + right_column_offset, y,
                                                        row_height, row_height,
                                                        default_color_button_style,
                                                        material->color_override,
                                                        "material_color_override_button", 0,
                                                        &color_override_button_id);
        Editor_Color_Picker *editor_color_picker = &editor_state->color_picker;
        if (ui_id_equals(editor_color_picker->parent_ui_id, color_override_button_id)) {
            push_layer(ui_manager);
            editor_color_picker->ui_state = do_color_picker(x + right_column_offset, y + row_height,
                                                            Editor_Constants::color_picker_style,
                                                            editor_color_picker->ui_state,
                                                            "editor_color_picker");
            pop_layer(ui_manager);
            material->color_override = make_vec4(rgb_to_vec3(editor_color_picker->ui_state.rgb_color), 1.0f);
            if (editor_color_picker->ui_state.should_hide) {
                editor_color_picker->parent_ui_id = {};
            }
        } else if (color_override_pressed) {
            open_color_picker(editor_color_picker, color_override_button_id, material->color_override);
        }

        y += row_height;
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);

        // USE COLOR OVERRIDE
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(x + padding_left, y, row_height,
                             "Use Color Override", editor_font_name, text_style);
        bool32 toggle_color_override_pressed = do_text_button(x + right_column_offset, y,
                                                              100.0f, row_height,
                                                              button_style, default_text_style,
                                                              material->use_color_override ? "true" : "false",
                                                              editor_font_name,
                                                              material->texture_id < 0,
                                                              "material_toggle_use_color_override");
        y += row_height;
        
        if (toggle_color_override_pressed) {
            material->use_color_override = !material->use_color_override;
            if (material->texture_id < 0 && !material->use_color_override) {
                material->use_color_override = true;
            }
        }
    }

    draw_row_padding(x, &y, row_width, padding_y, row_color,
                     side_flags | SIDE_BOTTOM, row_index++);

    if (entity->type == ENTITY_POINT_LIGHT) {
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);
        Point_Light_Entity *point_light = (Point_Light_Entity *) entity;

        // LIGHT COLOR
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(x + padding_left, y, row_height,
                             "Light Color", editor_font_name, text_style);
        UI_id point_light_color_button_id;
        bool32 point_light_color_pressed = do_color_button(x + right_column_offset, y,
                                                           row_height, row_height,
                                                           default_color_button_style,
                                                           make_vec4(point_light->light_color, 1.0f),
                                                           "point_light_color", 0,
                                                           &point_light_color_button_id);

        Editor_Color_Picker *editor_color_picker = &editor_state->color_picker;
        if (ui_id_equals(editor_color_picker->parent_ui_id, point_light_color_button_id)) {
            push_layer(ui_manager);
            editor_color_picker->ui_state = do_color_picker(x + right_column_offset, y + row_height,
                                                            Editor_Constants::color_picker_style,
                                                            editor_color_picker->ui_state,
                                                            "editor_color_picker");
            pop_layer(ui_manager);
            point_light->light_color = rgb_to_vec3(editor_color_picker->ui_state.rgb_color);
            if (editor_color_picker->ui_state.should_hide) {
                editor_color_picker->parent_ui_id = {};
            }
        } else if (point_light_color_pressed) {
            open_color_picker(editor_color_picker, point_light_color_button_id,
                              make_vec4(point_light->light_color, 1.0f));
        }

#if 0
        if (point_light_color_pressed) {
            open_color_picker(editor_state, Editor_Color_Picker::POINT_LIGHT_COLOR,
                              x + right_column_offset, y + row_height,
                              make_vec4(point_light->light_color, 1.0f),
                              &point_light->light_color,
                              { UI_COLOR_BUTTON, point_light_color_button_id, 0 });
        }
#endif
        
        y += row_height;

        // LIGHT MIN DISTANCE
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        buf = string_format(allocator, buffer_size, "%f", point_light->falloff_start);
        draw_v_centered_text(x + padding_left, y, row_height,
                             "Falloff Start", editor_font_name, text_style);
        point_light->falloff_start = do_slider(x+right_column_offset, y,
                                               edit_box_width, row_height,
                                               buf, editor_font_name,
                                               0.0f, 100.0f, point_light->falloff_start,
                                               default_slider_style, default_text_style,
                                               "edit_material_gloss_slider");
        y += row_height;
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);

        // LIGHT MAX DISTANCE
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        buf = string_format(allocator, buffer_size, "%f", point_light->falloff_end);
        draw_v_centered_text(x + padding_left, y, row_height,
                             "Falloff Distance", editor_font_name, text_style);
        point_light->falloff_end = do_slider(x+right_column_offset, y,
                                             edit_box_width, row_height,
                                             buf, editor_font_name,
                                             0.0f, 100.0f, point_light->falloff_end,
                                             default_slider_style, default_text_style,
                                             "edit_material_gloss_slider");
        y += row_height;
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags | SIDE_BOTTOM, row_index++);
    }
}

void draw_level_box(Game_State *game_state, Controller_State *controller_state,
                    real32 x, real32 y) {
    UI_Manager *ui_manager = &game_state->ui_manager;
    Editor_State *editor_state = &game_state->editor_state;
    
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

    draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags | SIDE_TOP, row_index);

    draw_row(x, y,
             row_width, button_height, row_color, side_flags, row_id, row_index++);

    Font font = get_font(game_state, editor_font_name);
    
    x += padding_x;
    real32 new_level_button_width = 50.0f;
    bool32 new_level_clicked = do_text_button(x, y,
                                              new_level_button_width, button_height,
                                              default_text_button_style, default_text_style,
                                              "New",
                                              editor_font_name, "new_level");
    if (new_level_clicked) {
        new_level(&game_state->current_level);
        editor_state->selected_entity_index = -1;
        editor_state->is_new_level = true;
    }

    x += new_level_button_width + 1;

    real32 open_level_button_width = 60.0f;
    bool32 open_level_clicked = do_text_button(x, y, 
                                               open_level_button_width, button_height,
                                               default_text_button_style, default_text_style,
                                               "Open",
                                               editor_font_name, "open_level");
    if (open_level_clicked) {
        Marker m = begin_region();
        char *absolute_filename = (char *) region_push(PLATFORM_MAX_PATH);
        
        if (platform_open_file_dialog(absolute_filename,
                                      LEVEL_FILE_FILTER_TITLE, LEVEL_FILE_FILTER_TYPE,
                                      PLATFORM_MAX_PATH)) {
            bool32 result = read_and_load_level(game_state,
                                                &game_state->current_level, absolute_filename,
                                                &memory.level_mesh_arena,
                                                &memory.level_string64_pool,
                                                &memory.level_filename_pool);
            if (result) {
                editor_state->is_new_level = false;
                copy_string(&editor_state->current_level_filename, make_string(absolute_filename));
                editor_state->selected_entity_index = -1;
            }
        }

        end_region(m);
    }

    y += button_height;
    x = initial_x;
    draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags, row_index);

    // level name text
    real32 label_row_height = 18.0f;
    draw_row(x, y, row_width, label_row_height, row_color, side_flags, row_id, row_index++);
    do_text(ui_manager, x + padding_x, y + get_center_baseline_offset(label_row_height, get_adjusted_font_height(font)),
            "Level Name", editor_font_name, default_text_style, "level_name_text_box_label");
    y += label_row_height;

    // level name text box
    draw_row(x, y, row_width, row_height, row_color, side_flags, row_id, row_index++);
    do_text_box(x + padding_x,
                y,
                row_width - padding_x*2, row_height,
                &game_state->current_level.name, editor_font_name,
                default_text_box_style, default_text_style,
                "level_name_text_box");
    y += row_height;
    draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags, row_index);

    // save level button
    draw_row(x, y, row_width, button_height, row_color, side_flags, row_id, row_index++);

    real32 save_button_width = 50.0f;
    bool32 save_level_clicked = do_text_button(x + padding_x, y,
                                               save_button_width, button_height,
                                               default_text_button_style, default_text_style,
                                               "Save",
                                               editor_font_name,
                                               is_empty(game_state->current_level.name),
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
                export_level((Allocator *) &memory.global_stack, game_state, &game_state->current_level, filename);
                copy_string(&editor_state->current_level_filename, make_string(filename));
                editor_state->is_new_level = false;
            }
        } else {
            char *level_filename = to_char_array((Allocator *) &memory.global_stack,
                                                 editor_state->current_level_filename);
            export_level((Allocator *) &memory.global_stack, game_state, &game_state->current_level, level_filename);
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
                                                  is_empty(game_state->current_level.name),
                                                  "save_as_level");

    if (save_as_level_clicked) {
        assert(!is_empty(game_state->current_level.name));

        Marker m = begin_region();
        char *filename = (char *) region_push(PLATFORM_MAX_PATH);

        bool32 has_filename = platform_open_save_file_dialog(filename,
                                                             LEVEL_FILE_FILTER_TITLE, LEVEL_FILE_FILTER_TYPE,
                                                             PLATFORM_MAX_PATH);

        if (has_filename) {
            export_level((Allocator *) &memory.global_stack, game_state, &game_state->current_level, filename);
            copy_string(&editor_state->current_level_filename, make_string(filename));
            editor_state->is_new_level = false;
        }
        
        end_region(m);
    }
    x = initial_x;

    y += button_height;
    
    draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags | SIDE_BOTTOM, row_index);
}

void draw_editor_ui(Game_State *game_state, Controller_State *controller_state) {
    Editor_State *editor_state = &game_state->editor_state;
    UI_Manager *ui_manager = &game_state->ui_manager;
    Render_State *render_state = &game_state->render_state;

    if (editor_state->selected_entity_index >= 0) {
        Entity *selected_entity = get_selected_entity(game_state);
        draw_entity_box(game_state, controller_state, selected_entity);

        if (editor_state->open_window_flags & MATERIAL_LIBRARY_WINDOW) {
            draw_material_library(game_state, controller_state, selected_entity);
        } else if (editor_state->open_window_flags & TEXTURE_LIBRARY_WINDOW) {
            Material *selected_material;
            bool32 material_exists = hash_table_find_pointer(game_state->current_level.material_table,
                                                             selected_entity->material_id,
                                                             &selected_material);
            assert(material_exists);
            draw_texture_library(selected_material);
        } else if (editor_state->open_window_flags & MESH_LIBRARY_WINDOW) {
            draw_mesh_library(game_state, controller_state, selected_entity);
        }
    } else {
        reset_entity_editors(editor_state);
    }

    real32 y = 0.0f;
    real32 button_gap = 1.0f;
    real32 sidebar_button_width = 200.0f;

    real32 button_height = 25.0f;
    char *button_font_name = editor_font_name;
    // wireframe toggle
    real32 wireframe_button_width = sidebar_button_width;
    bool32 toggle_show_wireframe_clicked = do_text_button(render_state->display_output.width - sidebar_button_width,
                                                          y,
                                                          wireframe_button_width, button_height,
                                                          default_text_button_style, default_text_style,
                                                          editor_state->show_wireframe ?
                                                          "Hide Wireframe" : "Show Wireframe",
                                                          button_font_name, "toggle_wireframe");
    if (toggle_show_wireframe_clicked) {
        editor_state->show_wireframe = !editor_state->show_wireframe;
    }
    y += button_height + button_gap;

    // transform mode toggle
    bool32 toggle_global_clicked = do_text_button(render_state->display_output.width - sidebar_button_width, y,
                                                  sidebar_button_width, button_height,
                                                  default_text_button_style, default_text_style,
                                                  editor_state->transform_mode == TRANSFORM_GLOBAL ?
                                                  "Use Local Transform" : "Use Global Transform",
                                                  button_font_name, "toggle_transform");
    if (toggle_global_clicked) {
        if (editor_state->transform_mode == TRANSFORM_GLOBAL) {
            editor_state->transform_mode = TRANSFORM_LOCAL;
        } else {
            editor_state->transform_mode = TRANSFORM_GLOBAL;
        }
    }
    y += button_height + button_gap;

    y += 5;
    draw_level_box(game_state, controller_state, render_state->display_output.width - 198.0f - 1.0f, y);

    y += 120.0f;

    bool32 add_entity_clicked = do_text_button(render_state->display_output.width - sidebar_button_width, y,
                                               sidebar_button_width, button_height,
                                               default_text_button_style, default_text_style,
                                               "Add Entity",
                                               button_font_name, "add_entity");

    if (add_entity_clicked) {
        int32 mesh_id = get_mesh_id_by_name(game_state,
                                            &game_state->current_level,
                                            Mesh_Type::PRIMITIVE,
                                            make_string("cube"));
        AABB primitive_cube_mesh_aabb = (get_mesh(game_state, &game_state->current_level,
                                                  Mesh_Type::PRIMITIVE, mesh_id)).aabb;
        
        Normal_Entity new_entity = make_entity(Mesh_Type::PRIMITIVE, mesh_id, -1, make_transform(),
                                               primitive_cube_mesh_aabb);
        int32 entity_id = level_add_entity(&game_state->current_level, new_entity);
        editor_state->selected_entity_type = ENTITY_NORMAL;
        editor_state->selected_entity_index = entity_id;
    }

    if (!editor_state->is_new_level) {
        char *filename_buf = to_char_array((Allocator *) &memory.frame_arena, editor_state->current_level_filename);
        char *buf = string_format((Allocator *) &memory.frame_arena, PLATFORM_MAX_PATH + 32,
                                  "current level: %s", filename_buf);
        do_text(ui_manager,
                5.0f, render_state->display_output.height - 9.0f,
                buf, editor_font_name, default_text_style, "editor_current_level_filename");
    }
}

int32 pick_entity(Game_State *game_state, Ray cursor_ray, Entity *entity_result, int32 *index_result) {
    Editor_State *editor_state = &game_state->editor_state;

    Level *level = &game_state->current_level;

    Hash_Table<int32, Mesh> mesh_table = level->mesh_table;
    Normal_Entity *normal_entities = level->normal_entities;
    Point_Light_Entity *point_lights = level->point_lights;

    Entity *picked_entity = NULL;
    int32 entity_index = -1;

    real32 t_min = FLT_MAX;
    for (int32 i = 0; i < level->num_normal_entities; i++) {
        real32 t, aabb_t;
        Normal_Entity *entity = &normal_entities[i];
        Mesh mesh = get_mesh(game_state, level, entity->mesh_type, entity->mesh_id);
        if (ray_intersects_aabb(cursor_ray, entity->transformed_aabb, &aabb_t) && (aabb_t < t_min)) {
            if (ray_intersects_mesh(cursor_ray, mesh, entity->transform, &t) && (t < t_min)) {
                t_min = t;
                entity_index = i;
                picked_entity = (Entity *) entity;
            }
        }
    }

    for (int32 i = 0; i < level->num_point_lights; i++) {
        real32 t, aabb_t;
        Point_Light_Entity *entity = &point_lights[i];
        Mesh mesh = get_mesh(game_state, level, entity->mesh_type, entity->mesh_id);
        if (ray_intersects_aabb(cursor_ray, entity->transformed_aabb, &aabb_t) && (aabb_t < t_min)) {
            if (ray_intersects_mesh(cursor_ray, mesh, entity->transform, &t) && (t < t_min)) {
                t_min = t;
                entity_index = i;
                picked_entity = (Entity *) entity;
            }
        }
    }

    if (entity_index >= 0) {
        *entity_result = *picked_entity;
        *index_result = entity_index;
        return true;
    }

    return false;
}

bool32 is_translation(Gizmo_Handle gizmo_axis) {
    return (gizmo_axis == GIZMO_TRANSLATE_X ||
            gizmo_axis == GIZMO_TRANSLATE_Y ||
            gizmo_axis == GIZMO_TRANSLATE_Z);
}

bool32 is_rotation(Gizmo_Handle gizmo_axis) {
    return (gizmo_axis == GIZMO_ROTATE_X ||
            gizmo_axis == GIZMO_ROTATE_Y ||
            gizmo_axis == GIZMO_ROTATE_Z);
}

Gizmo_Handle pick_gizmo(Game_State *game_state, Ray cursor_ray,
                        Vec3 *gizmo_initial_hit, Vec3 *gizmo_transform_axis) {
    Editor_State *editor_state = &game_state->editor_state;
    Entity *entity = get_selected_entity(game_state);
    Gizmo gizmo = editor_state->gizmo;

    Transform_Mode transform_mode = editor_state->transform_mode;

    Mat4 entity_model_matrix = get_model_matrix(entity->transform);

    Transform x_transform, y_transform, z_transform;
    Vec3 transform_x_axis, transform_y_axis, transform_z_axis;
    // TODO: maybe add some scale here? for a bit of extra space to click on the gizmo.
    if (transform_mode == TRANSFORM_GLOBAL) {
        x_transform = gizmo.transform;
        x_transform.rotation = make_quaternion();
        y_transform = gizmo.transform;
        y_transform.rotation = make_quaternion(90.0f, z_axis);
        z_transform = gizmo.transform;
        z_transform.rotation = make_quaternion(-90.0f, y_axis);

        transform_x_axis = x_axis;
        transform_y_axis = y_axis;
        transform_z_axis = z_axis;
    } else {
        // local transform
        x_transform = gizmo.transform;
        y_transform = gizmo.transform;
        y_transform.rotation = gizmo.transform.rotation*make_quaternion(90.0f, z_axis);
        z_transform = gizmo.transform;
        z_transform.rotation = gizmo.transform.rotation*make_quaternion(-90.0f, y_axis);

        transform_x_axis = normalize(truncate_v4_to_v3(entity_model_matrix.col1));
        transform_y_axis = normalize(truncate_v4_to_v3(entity_model_matrix.col2));
        transform_z_axis = normalize(truncate_v4_to_v3(entity_model_matrix.col3));
    }

    Gizmo_Handle picked_handle = GIZMO_HANDLE_NONE;

    Transform gizmo_handle_transforms[3] = { x_transform, y_transform, z_transform };
    Vec3 transform_axes[3] = { transform_x_axis, transform_y_axis, transform_z_axis };
    Vec3 selected_transform_axis = transform_x_axis;

    // check ray against translation arrows
    Gizmo_Handle gizmo_translation_handles[3] = { GIZMO_TRANSLATE_X, GIZMO_TRANSLATE_Y, GIZMO_TRANSLATE_Z };
    assert(gizmo.arrow_mesh_id >= 0);
    Mesh arrow_mesh = get_common_mesh(game_state, gizmo.arrow_mesh_id);

    real32 t_min = FLT_MAX;
    for (int32 i = 0; i < 3; i++) {
        Transform gizmo_handle_transform = gizmo_handle_transforms[i];
        real32 t;
        if (ray_intersects_mesh(cursor_ray, arrow_mesh, gizmo_handle_transform, &t) && (t < t_min)) {
            t_min = t;
            picked_handle = gizmo_translation_handles[i];
            selected_transform_axis = transform_axes[i];
        }
    }

    // check ray against rotation rings
    Gizmo_Handle gizmo_rotation_handles[3] = { GIZMO_ROTATE_X, GIZMO_ROTATE_Y, GIZMO_ROTATE_Z };
    assert(gizmo.ring_mesh_id >= 0);
    Mesh ring_mesh = get_common_mesh(game_state, gizmo.ring_mesh_id);

    for (int32 i = 0; i < 3; i++) {
        Transform gizmo_handle_transform = gizmo_handle_transforms[i];
        real32 t;
        if (ray_intersects_mesh(cursor_ray, ring_mesh, gizmo_handle_transform, &t) && (t < t_min)) {
            t_min = t;
            picked_handle = gizmo_rotation_handles[i];
            selected_transform_axis = transform_axes[i];
        }
    }

    if (picked_handle) {
        if (is_rotation(picked_handle)) {
            real32 t;
            Plane plane = { dot(gizmo.transform.position, selected_transform_axis),
                            selected_transform_axis };
            bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);
            if (intersects_plane) {
                *gizmo_initial_hit = cursor_ray.origin + t*cursor_ray.direction;
            } else {
                *gizmo_initial_hit = cursor_ray.origin + t_min*cursor_ray.direction;;
            }
        } else {
            *gizmo_initial_hit = cursor_ray.origin + t_min*cursor_ray.direction;
        }

        *gizmo_transform_axis = selected_transform_axis;
    }

    return picked_handle;
}

Vec3 do_gizmo_translation(Camera *camera, Editor_State *editor_state, Ray cursor_ray) {
    real32 t;
    Vec3 camera_forward = camera->current_basis.forward;
    Ray transform_ray = make_ray(editor_state->gizmo_initial_hit,
                                 editor_state->gizmo_transform_axis);
    Plane plane = get_plane_containing_ray(transform_ray, camera_forward);
    bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);

    Vec3 delta_result = make_vec3();

    // this will always intersect unless your FOV is >= 180 degrees
    if (intersects_plane) {
        Vec3 intersect_point = cursor_ray.origin + t*cursor_ray.direction;
        real32 delta_length = dot(intersect_point - editor_state->last_gizmo_transform_point,
                                  editor_state->gizmo_transform_axis);
        Vec3 delta = editor_state->gizmo_transform_axis * delta_length;
        editor_state->last_gizmo_transform_point += delta;
        delta_result = delta;
    }

    return delta_result;
}

Quaternion do_gizmo_rotation(Camera *camera, Editor_State *editor_state, Ray cursor_ray) {
    real32 t;
    Vec3 camera_forward = camera->current_basis.forward;
    Vec3 center = editor_state->gizmo.transform.position;
    Plane plane = { dot(center, editor_state->gizmo_transform_axis),
                    editor_state->gizmo_transform_axis };
    bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);

    Quaternion delta_result = make_quaternion();

    // this will always intersect unless your FOV is >= 180 degrees
    if (intersects_plane) {
        Vec3 intersect_point = cursor_ray.origin + t*cursor_ray.direction;

        Vec3 center_to_intersect_point = intersect_point - center;
        Vec3 center_to_last_intersect_point = editor_state->last_gizmo_transform_point - center;

        if (are_collinear(center_to_last_intersect_point, center_to_intersect_point)) {
            return delta_result;
        }

        Vec3 out_vector = cross(editor_state->gizmo_transform_axis, center_to_last_intersect_point);
        
        real32 sign = 1.0f;
        if (dot(center_to_intersect_point, out_vector) < 0.0f) sign = -1.0f;

        real32 a = distance(center_to_intersect_point);
        real32 b = distance(center_to_last_intersect_point);
        real32 c = distance(intersect_point - editor_state->last_gizmo_transform_point);

        real32 angle_delta_degrees = sign * cosine_law_degrees(a, b, c);
        Quaternion delta = make_quaternion(angle_delta_degrees, editor_state->gizmo_transform_axis);

        editor_state->last_gizmo_transform_point = intersect_point;
        delta_result = delta;
    }

    return delta_result;
}

void update_editor_camera(Camera *camera, Controller_State *controller_state,
                          bool32 use_freecam, bool32 has_focus, bool32 should_move,
                          real32 dt) {
    Basis initial_basis = camera->initial_basis;

    if (use_freecam && has_focus) {
        real32 delta_x = controller_state->current_mouse.x - controller_state->last_mouse.x;
        real32 delta_y = controller_state->current_mouse.y - controller_state->last_mouse.y;

        real32 heading_delta = 0.2f * delta_x;
        real32 pitch_delta = 0.2f * delta_y;

        int32 heading_rotations = (int32) floorf((camera->heading + heading_delta) / 360.0f);
        int32 pitch_rotations = (int32) floorf((camera->pitch + pitch_delta) / 360.0f);
        camera->heading = (camera->heading + heading_delta) - heading_rotations*360.0f;
        camera->pitch = clamp(camera->pitch + pitch_delta, -90.0f, 90.0f);
    }
    
    Mat4 model_matrix = make_rotate_matrix(camera->roll, camera->pitch, camera->heading);
    Vec3 transformed_forward = truncate_v4_to_v3(model_matrix * make_vec4(initial_basis.forward, 1.0f));
    Vec3 transformed_right = truncate_v4_to_v3(model_matrix * make_vec4(initial_basis.right, 1.0f));
    Vec3 transformed_up = cross(transformed_forward, transformed_right);
    // we calculate a new right vector to correct for any error to ensure that our vectors form an
    // orthonormal basis
    Vec3 corrected_right = cross(transformed_up, transformed_forward);

    Vec3 forward = normalize(transformed_forward);
    Vec3 right = normalize(corrected_right);
    Vec3 up = normalize(transformed_up);

    if (should_move) {
        Vec3 movement_delta = make_vec3();
        if (controller_state->key_w.is_down) {
            movement_delta += forward;
        }
        if (controller_state->key_s.is_down) {
            movement_delta -= forward;
        }
        if (controller_state->key_d.is_down) {
            movement_delta += right;
        }
        if (controller_state->key_a.is_down) {
            movement_delta -= right;
        }

        real32 camera_speed = Editor_Constants::camera_speed;
        movement_delta = normalize(movement_delta) * camera_speed;
        camera->position += movement_delta * dt;
    }
    
    Basis current_basis = { forward, right, up };
    camera->current_basis = current_basis;
}

void update_gizmo(Game_State *game_state) {
    Editor_State *editor_state = &game_state->editor_state;
    if (editor_state->selected_entity_index < 0) return;

    Camera *camera = &game_state->render_state.camera;
    real32 gizmo_scale_factor = distance(editor_state->gizmo.transform.position - camera->position) /
        5.0f;
    editor_state->gizmo.transform.scale = make_vec3(gizmo_scale_factor, gizmo_scale_factor, gizmo_scale_factor);

    Entity *entity = get_selected_entity(game_state);
    editor_state->gizmo.transform.position = entity->transform.position;
    editor_state->gizmo.transform.rotation = entity->transform.rotation;
}


void update_editor(Game_State *game_state, Controller_State *controller_state, real32 dt) {
    UI_Manager *ui_manager = &game_state->ui_manager;
    Editor_State *editor_state = &game_state->editor_state;
    Render_State *render_state = &game_state->render_state;
    Display_Output *display_output = &game_state->render_state.display_output;

    if (just_pressed(controller_state->key_tab) && !has_focus(ui_manager)) {
        editor_state->use_freecam = !editor_state->use_freecam;
        platform_set_cursor_visible(!editor_state->use_freecam);
    }

    bool32 camera_should_move = editor_state->use_freecam && !has_focus(ui_manager);
    update_editor_camera(&render_state->camera, controller_state, editor_state->use_freecam,
                         platform_window_has_focus(), camera_should_move, dt);
    update_render_state(render_state);

    if (editor_state->use_freecam && platform_window_has_focus()) {
        Vec2 center = make_vec2(display_output->width / 2.0f, display_output->height / 2.0f);
        platform_set_cursor_pos(center);
        controller_state->current_mouse = center;
    }

    if (editor_state->use_freecam) {
        disable_input(ui_manager);
    } else {
        enable_input(ui_manager);
    }
    
    // mesh picking
    Vec3 cursor_world_space = cursor_pos_to_world_space(controller_state->current_mouse,
                                                        &game_state->render_state);
    Ray cursor_ray = { cursor_world_space,
                       normalize(cursor_world_space - render_state->camera.position) };
    
    if (!ui_has_hot(ui_manager) &&
        !ui_has_active(ui_manager) &&
        !editor_state->use_freecam && was_clicked(controller_state->left_mouse)) {
        if (!editor_state->selected_gizmo_handle) {
            Entity entity;
            int32 entity_index;
            bool32 picked = pick_entity(game_state, cursor_ray, &entity, &entity_index);
            
            if (picked) {
                if (selected_entity_changed(editor_state, entity_index, entity.type)) {
                    editor_state->last_selected_entity_type = editor_state->selected_entity_type;
                    editor_state->last_selected_entity_index = editor_state->selected_entity_index;

                    editor_state->selected_entity_type = entity.type;
                    editor_state->selected_entity_index = entity_index;

                    editor_state->gizmo.transform = entity.transform;

                    reset_entity_editors(editor_state);
                }
            } else {
                editor_state->selected_entity_index = -1;
            }
        }
    }

    update_gizmo(game_state);

    if (editor_state->selected_entity_index >= 0 &&
        !ui_has_hot(ui_manager) &&
        !editor_state->selected_gizmo_handle) {

        Vec3 gizmo_initial_hit, gizmo_transform_axis;
        Gizmo_Handle picked_handle = pick_gizmo(game_state, cursor_ray,
                                                &gizmo_initial_hit, &gizmo_transform_axis);
        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
            editor_state->selected_gizmo_handle = picked_handle;
            editor_state->gizmo_initial_hit = gizmo_initial_hit;
            editor_state->gizmo_transform_axis = gizmo_transform_axis;
            editor_state->last_gizmo_transform_point = gizmo_initial_hit;
        } else {
            editor_state->hovered_gizmo_handle = picked_handle;
        }
    }

    if (editor_state->use_freecam ||
        (ui_has_hot(ui_manager) && !controller_state->left_mouse.is_down)) {
        editor_state->hovered_gizmo_handle = GIZMO_HANDLE_NONE;
        editor_state->selected_gizmo_handle = GIZMO_HANDLE_NONE;
    }
    
    if (editor_state->selected_gizmo_handle) {
        disable_input(ui_manager);
        if (controller_state->left_mouse.is_down) {
            Entity *entity = get_selected_entity(game_state);

            if (is_translation(editor_state->selected_gizmo_handle)) {
                Vec3 delta = do_gizmo_translation(&render_state->camera, editor_state, cursor_ray);
                update_entity_position(game_state, entity, entity->transform.position + delta);
                
            } else if (is_rotation(editor_state->selected_gizmo_handle)) {
                Quaternion delta = do_gizmo_rotation(&render_state->camera, editor_state, cursor_ray);
                update_entity_rotation(game_state, entity, delta*entity->transform.rotation);
            }

            editor_state->gizmo.transform.position = entity->transform.position;
            editor_state->gizmo.transform.rotation = entity->transform.rotation;
        } else {
            editor_state->selected_gizmo_handle = GIZMO_HANDLE_NONE;
        }
    }

    update_gizmo(game_state);
}
