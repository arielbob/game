#include "font.h"
#include "editor.h"
#include "game.h"

#define SIDE_LEFT   0x1
#define SIDE_RIGHT  0x2
#define SIDE_TOP    0x4
#define SIDE_BOTTOM 0x8

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

char *editor_font_name = "courier18";
char *editor_font_name_bold = "courier18b";

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
    //editor_state->editing_selected_entity_material = false;
}

void draw_row_line(UI_Manager *ui_manager, Controller_State *controller_state,
                   real32 x, real32 *y,
                   real32 row_width, bool32 draw_outside_row=true) {
    Vec4 line_color = make_vec4(0.3f, 0.3f, 0.3f, 1.0f);
    UI_Box_Style line_box_style = { line_color };
    real32 line_thickness = 1.0f;
    char *box_id = "row_line";
    if (draw_outside_row) {
        x -= 1.0f;
        row_width += 2.0f;
    }
    do_box(ui_manager, controller_state, x, *y, row_width, line_thickness,
           line_box_style, box_id);
    *y += 1;
}

void draw_row(UI_Manager *ui_manager, Controller_State *controller_state,
              real32 x, real32 y,
              real32 row_width, real32 row_height,
              Vec4 color, uint8 side_flags,
              bool32 inside_border,
              char *row_id, int32 index) {
    UI_Box_Style box_style = { color };

    do_box(ui_manager, controller_state, x, y, row_width, row_height,
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
        do_box(ui_manager, controller_state, box_x, y, line_thickness, row_height,
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
        do_box(ui_manager, controller_state, box_x, box_y, box_width, line_thickness,
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
        do_box(ui_manager, controller_state, box_x, box_y, box_width, line_thickness,
               line_box_style, border_id);
    }
    if (side_flags & SIDE_RIGHT) {
        real32 box_x = x + row_width - 1;
        if (!inside_border) box_x += 1;
        do_box(ui_manager, controller_state, box_x, y, line_thickness, row_height,
               line_box_style, border_id);
    }
}

inline void draw_row(UI_Manager *ui_manager, Controller_State *controller_state,
                     real32 x, real32 y,
                     real32 row_width, real32 row_height,
                     Vec4 color, uint8 side_flags,
                     char *row_id, int32 index) {
    return draw_row(ui_manager, controller_state,
                    x, y,
                    row_width, row_height,
                    color, side_flags,
                    false,
                    row_id, index);
}

inline void draw_row_padding(UI_Manager *ui_manager, Controller_State *controller_state,
                             real32 x, real32 *y,
                             real32 row_width,
                             real32 padding,
                             Vec4 color, uint8 side_flags,
                             int32 index) {
    draw_row(ui_manager, controller_state,
             x, *y,
             row_width, padding,
             color, side_flags,
             "row_padding", index);
    *y += padding;
    if (side_flags & SIDE_BOTTOM) {
        // beacuse we're drawing the border on the outside, we need to offset by the border thickness (1)
        *y += 1;
    }
}

void draw_centered_text(Game_State *game_state, UI_Manager *ui,
                        real32 box_x, real32 box_y,
                        real32 row_width, real32 row_height,
                        char *text, char *font_name, UI_Text_Style text_style) {
    Font font = get_font(game_state, font_name);
    real32 adjusted_text_height = font.height_pixels - font.scale_for_pixel_height * (font.ascent + font.descent);
    real32 x_offset = 0.5f * row_width - 0.5f * get_width(font, text);
    real32 y_offset = 0.5f * (row_height + adjusted_text_height);
    do_text(ui, box_x + x_offset, box_y + y_offset,
            text, font_name, text_style, "entity_properties_text");
}

void draw_v_centered_text(Game_State *game_state, UI_Manager *ui,
                          real32 box_x, real32 box_y,
                          real32 row_height,
                          char *text, char *font_name, UI_Text_Style text_style) {
    Font font = get_font(game_state, font_name);
    real32 adjusted_text_height = font.height_pixels - font.scale_for_pixel_height * (font.ascent + font.descent);
    real32 y_offset = 0.5f * (row_height + adjusted_text_height);
    do_text(ui, box_x, box_y + y_offset,
            text, font_name, text_style, "entity_properties_text");
}

void draw_labeled_text(Game_State *game_state, UI_Manager *ui_manager,
                       real32 x, real32 y,
                       real32 row_height,
                       char *label_font, char *label,
                       char *text_font, char *text,
                       UI_Text_Style text_style) {
    real32 small_spacing = 20.0f;
    draw_v_centered_text(game_state, ui_manager, x, y, row_height,
                         label, label_font, text_style);
    draw_v_centered_text(game_state, ui_manager, x+small_spacing, y, row_height,
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

    char *row_id = "mesh_library_row";
    int32 row_index = 0;

    real32 initial_x = render_state->display_output.width / 2.0f - window_width / 2.0f;
    real32 x = initial_x;
    real32 y = 80.0f;

    UI_Text_Style text_style = default_text_style;

    draw_row(ui_manager, controller_state, x, y, window_width, title_row_height, title_row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_TOP | SIDE_BOTTOM, row_id, row_index++);
    draw_centered_text(game_state, ui_manager, x, y, window_width, title_row_height,
                       "Mesh Library", editor_font_name_bold, text_style);
    y += title_row_height;

    real32 content_height = 500.0f;
    draw_row(ui_manager, controller_state, x, y, window_width, content_height, row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_BOTTOM, row_id, row_index++);    

    real32 cancel_button_width = 100.0f;
    bool32 cancel_pressed = do_text_button(ui_manager, controller_state,
                                           x + padding_x,
                                           y + content_height - SMALL_ROW_HEIGHT - padding_y - 1,
                                           cancel_button_width, SMALL_ROW_HEIGHT,
                                           default_text_button_cancel_style, default_text_style,
                                           "Cancel",
                                           editor_font_name_bold,
                                           "choose_mesh_cancel");
    if (cancel_pressed) {
        editor_state->open_window_flags = 0;
    }

    Allocator *allocator = (Allocator *) &memory.frame_arena;

    x += padding_x;
    y += padding_y;

    UI_Image_Button_Style image_button_style = default_image_button_style;
    image_button_style.image_constraint_flags = CONSTRAINT_FILL_BUTTON_WIDTH | CONSTRAINT_KEEP_IMAGE_PROPORTIONS;

    Hash_Table<int32, Mesh> *mesh_table = &game_state->mesh_table;
    char *button_id_string = "mesh_library_item";
    int32 picked_mesh_id = -1;
    for (int32 i = 0; i < mesh_table->max_entries; i++) {
        Hash_Table_Entry<int32, Mesh> *entry = &mesh_table->entries[i];
        if (!entry->is_occupied) continue;

        Mesh *mesh = &entry->value;
        char *mesh_name = to_char_array(allocator, mesh->name);
        bool32 pressed = do_text_button(ui_manager, controller_state,
                                        x, y,
                                        item_width, item_height,
                                        default_text_button_style, default_text_style,
                                        mesh_name,
                                        editor_font_name_bold,
                                        button_id_string, i);

        if (pressed) picked_mesh_id = entry->key;
        x += item_width + x_gap;
        if (x + item_width > initial_x + window_width) {
            x = initial_x + padding_x;
            y += item_height + y_gap;
        }
    }
    
    if (picked_mesh_id >= 0) {
        if (selected_entity->mesh_id < 0 ||
            picked_mesh_id != selected_entity->mesh_id) {
            selected_entity->mesh_id = picked_mesh_id;
        }

        editor_state->open_window_flags = 0;
    }

    pop_layer(ui_manager);
}

void draw_texture_library(Game_State *game_state, Controller_State *controller_state, Material *selected_material) {
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

    char *row_id = "texture_library_row";
    int32 row_index = 0;

    real32 initial_x = render_state->display_output.width / 2.0f - window_width / 2.0f;
    real32 x = initial_x;
    real32 y = 80.0f;

    UI_Text_Style text_style = default_text_style;

    draw_row(ui_manager, controller_state, x, y, window_width, title_row_height, title_row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_TOP | SIDE_BOTTOM, row_id, row_index++);
    draw_centered_text(game_state, ui_manager, x, y, window_width, title_row_height,
                       "Texture Library", editor_font_name_bold, text_style);
    y += title_row_height;

    real32 content_height = 500.0f;
    draw_row(ui_manager, controller_state, x, y, window_width, content_height, row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_BOTTOM, row_id, row_index++);    

    real32 cancel_button_width = 100.0f;
    bool32 cancel_pressed = do_text_button(ui_manager, controller_state,
                                           x + padding_x,
                                           y + content_height - SMALL_ROW_HEIGHT - padding_y - 1,
                                           cancel_button_width, SMALL_ROW_HEIGHT,
                                           default_text_button_cancel_style, default_text_style,
                                           "Cancel",
                                           editor_font_name_bold,
                                           "choose_texture_cancel");
    if (cancel_pressed) {
        editor_state->open_window_flags = 0;
    }

    Allocator *allocator = (Allocator *) &memory.frame_arena;

    x += padding_x;
    y += padding_y;

    UI_Image_Button_Style image_button_style = default_image_button_style;
    image_button_style.image_constraint_flags = CONSTRAINT_FILL_BUTTON_WIDTH | CONSTRAINT_KEEP_IMAGE_PROPORTIONS;

    Hash_Table<int32, Texture> *texture_table = &game_state->texture_table;
    char *button_id_string = "texture_library_item";
    int32 picked_texture_id = -1;
    for (int32 i = 0; i < texture_table->max_entries; i++) {
        Hash_Table_Entry<int32, Texture> *entry = &texture_table->entries[i];
        if (!entry->is_occupied) continue;

        Texture *texture = &entry->value;
        char *texture_name = to_char_array(allocator, texture->name);
        bool32 pressed = do_image_button(ui_manager, controller_state,
                                         x, y,
                                         item_width, item_height,
                                         image_button_style, default_text_style,
                                         entry->key, texture_name, editor_font_name_bold,
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

    draw_row(ui_manager, controller_state, x, y, window_width, title_row_height, title_row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_TOP | SIDE_BOTTOM, row_id, row_index++);
    draw_centered_text(game_state, ui_manager, x, y, window_width, title_row_height,
                       "Material Library", editor_font_name_bold, text_style);
    y += title_row_height;

    real32 content_height = 500.0f;
    draw_row(ui_manager, controller_state, x, y, window_width, content_height, row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_BOTTOM, row_id, row_index++);    

    real32 cancel_button_width = 100.0f;
    bool32 cancel_pressed = do_text_button(ui_manager, controller_state,
                                           x + padding_x,
                                           y + content_height - SMALL_ROW_HEIGHT - padding_y - 1,
                                           cancel_button_width, SMALL_ROW_HEIGHT,
                                           default_text_button_cancel_style, default_text_style,
                                           "Cancel",
                                           editor_font_name_bold,
                                           "choose_material_cancel");
    if (cancel_pressed) {
        editor_state->open_window_flags = 0;
    }
    
    Allocator *allocator = (Allocator *) &memory.frame_arena;

    x += padding_x;
    y += padding_y;

    Hash_Table<int32, Material> *material_table = &game_state->material_table;
    char *button_id_string = "material_library_item";
    int32 picked_material_id = -1;
    for (int32 i = 0; i < material_table->max_entries; i++) {
        Hash_Table_Entry<int32, Material> *entry = &material_table->entries[i];
        if (!entry->is_occupied) continue;

        Material *material = &entry->value;
        bool32 pressed = do_text_button(ui_manager, controller_state,
                                        x, y,
                                        item_width, item_height,
                                        default_text_button_style, default_text_style,
                                        to_char_array(allocator, material->name),
                                        editor_font_name_bold,
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

void draw_entity_box(Game_State *game_state, Controller_State *controller_state, Entity *entity) {
    int32 row_index = 0;

    UI_Manager *ui_manager = &game_state->ui_manager;
    Editor_State *editor_state = &game_state->editor_state;

    real32 box_x = 5.0f;
    real32 box_y = 50.0f;

    real32 box_padding_x = 10.0f;
    real32 box_padding_y = 10.0f;

    Allocator *allocator = (Allocator *) &memory.frame_arena;

    Mesh *mesh = get_mesh_pointer(game_state, entity->mesh_id);
    Material *material;
    bool32 material_exists = hash_table_find_pointer(game_state->material_table,
                                                     entity->material_id,
                                                     &material);
    assert(material_exists);
    Transform transform = entity->transform;

    UI_Text_Button_Style button_style = default_text_button_style;
    
    UI_Text_Style text_style = default_text_style;
    
    real32 padding_left = 6.0f;
    real32 padding_right = padding_left;
    real32 padding_top = padding_left;
    real32 padding_bottom = padding_left;
    real32 padding_y = padding_top;
    real32 right_column_offset = padding_left + 200.0f;
    real32 small_spacing = 20.0f;

    real32 initial_row_height = 22.0f;
    real32 row_height = initial_row_height;
    real32 small_row_height = SMALL_ROW_HEIGHT;
    real32 row_width = 500.0f;

    Vec4 title_row_color = make_vec4(0.05f, 0.2f, 0.5f, 1.0f);
    Vec4 row_color = make_vec4(0.1f, 0.1f, 0.1f, 0.9f);

    real32 initial_x = box_x;
    real32 x = box_x;
    real32 y = box_y;

    real32 title_row_height = 30.0f;
    uint8 side_flags = SIDE_LEFT | SIDE_RIGHT;

    char *row_id = "mesh_properties_line";

    draw_row(ui_manager, controller_state, x, y, row_width, title_row_height, title_row_color,
             side_flags | SIDE_TOP, row_id, row_index++);
    draw_centered_text(game_state, ui_manager, x, y, row_width, title_row_height,
                       "Entity Properties", editor_font_name_bold, text_style);
    y += title_row_height;
    draw_row_line(ui_manager, controller_state, x, &y, row_width);
        
    real32 choose_mesh_button_width = 200.0f;
    real32 add_mesh_button_width = 30.0f;
    choose_mesh_button_width -= add_mesh_button_width;

    char *mesh_name = to_char_array(allocator, mesh->name);
    draw_row_padding(ui_manager, controller_state, x, &y, row_width, padding_top, row_color, side_flags,
                     row_index++);
    draw_row(ui_manager, controller_state, x, y, row_width, row_height, row_color, side_flags,
             row_id, row_index++);
    draw_v_centered_text(game_state, ui_manager, x + padding_left, y, row_height,
                         "Mesh", editor_font_name_bold, text_style);
    x += right_column_offset;
    bool32 choose_mesh_pressed = do_text_button(ui_manager, controller_state,
                                                x, y,
                                                choose_mesh_button_width, row_height,
                                                button_style, default_text_style,
                                                to_char_array(allocator, mesh->name),
                                                editor_font_name_bold, "choose_mesh");

    if (!editor_state->open_window_flags && choose_mesh_pressed) {
        editor_state->open_window_flags |= MESH_LIBRARY_WINDOW;
    }
    x += choose_mesh_button_width + padding_left;

    bool32 add_mesh_pressed = do_text_button(ui_manager, controller_state,
                                             x, y,
                                             add_mesh_button_width, row_height,
                                             button_style, default_text_style,
                                             "+", editor_font_name_bold, "add_mesh");
    x += add_mesh_button_width + padding_left;

    if (add_mesh_pressed) {
        Pool_Allocator *string64_pool = &memory.string64_pool;

        // TODO: open file dialog and stuff
/*
        Mesh new_mesh = { 
            make_string_buffer((Allocator *) string64_pool, "New Mesh", MATERIAL_STRING_MAX_SIZE),
            -1,
            50.0f,
            make_vec4(0.0f, 0.0f, 0.0f, 1.0f),
            true
        };

        int32 mesh_id = add_mesh(game_state, new_mesh);
        entity->mesh_id = mesh_id;
*/
    }

    real32 edit_mesh_button_width = row_width - (x - initial_x) - padding_right;
    bool32 edit_mesh_pressed = do_text_button(ui_manager, controller_state,
                                              x, y,
                                              edit_mesh_button_width, row_height,
                                              button_style, default_text_style,
                                              "Edit", editor_font_name_bold,
                                              false,
                                              "edit_mesh");

    if (edit_mesh_pressed) {
        editor_state->editing_selected_entity_mesh = !editor_state->editing_selected_entity_mesh;
    }

    y += row_height;
    x = initial_x;
    if (editor_state->editing_selected_entity_mesh) {
        draw_row_padding(ui_manager, controller_state, x, &y, row_width, padding_y, row_color, side_flags,
            row_index++);
        UI_Text_Box_Style text_box_style = default_text_box_style;
        real32 edit_box_width = choose_mesh_button_width + padding_left + add_mesh_button_width;

        draw_row(ui_manager, controller_state, x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(game_state, ui_manager, x + padding_left, y, row_height,
                             "Mesh Name", editor_font_name_bold, default_text_style);
        do_text_box(ui_manager, controller_state,
                    x + right_column_offset,
                    y,
                    edit_box_width, row_height,
                    &mesh->name, editor_font_name_bold,
                    text_box_style, default_text_style,
                    "mesh_name_text_box");
        y += row_height;
        
    }
    draw_row_padding(ui_manager, controller_state, x, &y, row_width, padding_y, row_color,
                     side_flags | SIDE_BOTTOM, row_index++);

    x = initial_x;

    char *buf;
    int32 buffer_size = 16;
    
    // POSITION
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    draw_v_centered_text(game_state, ui_manager, x+padding_left, y, small_row_height,
                         "Position", editor_font_name_bold, text_style);
    // x
    buf = string_format(allocator, buffer_size, "%f", transform.position.x);
    draw_labeled_text(game_state, ui_manager, x+right_column_offset, y, small_row_height,
                      editor_font_name_bold, "x", editor_font_name, buf, text_style);
    y += small_row_height;
    // y
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.position.y);
    draw_labeled_text(game_state, ui_manager, x + right_column_offset, y, small_row_height,
                      editor_font_name_bold, "y", editor_font_name, buf, text_style);
    y += small_row_height;
    // z
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags,
             row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.position.z);
    draw_labeled_text(game_state, ui_manager, x + right_column_offset, y, small_row_height,
                      editor_font_name_bold, "z", editor_font_name, buf, text_style);
    y += small_row_height;    

    draw_row_line(ui_manager, controller_state, x, &y, row_width);

    // ROTATION
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    draw_v_centered_text(game_state, ui_manager, x+padding_left, y, small_row_height,
                         "Rotation", editor_font_name_bold, text_style);
    // w
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.w);
    draw_labeled_text(game_state, ui_manager, x + right_column_offset, y, small_row_height,
                      editor_font_name_bold, "w", editor_font_name, buf, text_style);
    y += small_row_height;
    // x
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.v.x);
    draw_labeled_text(game_state, ui_manager, x + right_column_offset, y, small_row_height,
                      editor_font_name_bold, "x", editor_font_name, buf, text_style);
    y += small_row_height;
    // y
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.v.y);
    draw_labeled_text(game_state, ui_manager, x + right_column_offset, y, small_row_height,
                      editor_font_name_bold, "y", editor_font_name, buf, text_style);
    y += small_row_height;
    // z
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags,
             row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.v.z);
    draw_labeled_text(game_state, ui_manager, x + right_column_offset, y, small_row_height,
                      editor_font_name_bold, "z", editor_font_name, buf, text_style);
    y += small_row_height;

    draw_row_line(ui_manager, controller_state, x, &y, row_width);

    // SCALE
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    draw_v_centered_text(game_state, ui_manager, x+padding_left, y, small_row_height,
                         "Scale", editor_font_name_bold, text_style);
    // x
    buf = string_format(allocator, buffer_size, "%f", transform.scale.x);
    draw_labeled_text(game_state, ui_manager, x + right_column_offset, y, small_row_height,
                      editor_font_name_bold, "x", editor_font_name, buf, text_style);
    y += small_row_height;
    // y
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.scale.y);
    draw_labeled_text(game_state, ui_manager, x + right_column_offset, y, small_row_height,
                      editor_font_name_bold, "y", editor_font_name, buf, text_style);
    y += small_row_height;
    // z
    draw_row(ui_manager, controller_state, x, y, row_width, small_row_height, row_color, side_flags | SIDE_BOTTOM,
             row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.scale.z);
    draw_labeled_text(game_state, ui_manager, x + right_column_offset, y, small_row_height,
                      editor_font_name_bold, "z", editor_font_name, buf, text_style);
    y += small_row_height;

    draw_row_line(ui_manager, controller_state, x, &y, row_width);

    // MATERIAL
    draw_row_padding(ui_manager, controller_state, x, &y, row_width, padding_bottom, row_color,
                     side_flags, row_index++);

    draw_row(ui_manager, controller_state, x, y, row_width, row_height, row_color, side_flags,
             row_id, row_index++);
    draw_v_centered_text(game_state, ui_manager, x+padding_left, y, row_height,
                         "Material", editor_font_name_bold, text_style);

    side_flags = SIDE_LEFT | SIDE_RIGHT;

    real32 choose_material_button_width = 200.0f;
    real32 add_material_button_width = 30.0f;
    choose_material_button_width -= add_material_button_width;

    x += right_column_offset;
    bool32 choose_material_pressed = do_text_button(ui_manager, controller_state,
                                                    x, y,
                                                    choose_material_button_width, row_height,
                                                    button_style, default_text_style,
                                                    to_char_array(allocator, material->name),
                                                    editor_font_name_bold, "choose_material");

    if (!editor_state->open_window_flags && choose_material_pressed) {
        editor_state->open_window_flags |= MATERIAL_LIBRARY_WINDOW;
    }
    x += choose_material_button_width + padding_left;

    bool32 add_material_pressed = do_text_button(ui_manager, controller_state,
                                                 x, y,
                                                 add_material_button_width, row_height,
                                                 button_style, default_text_style,
                                                 "+", editor_font_name_bold, "add_material");
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

        int32 material_id = add_material(game_state, new_material);
        entity->material_id = material_id;
        editor_state->editing_selected_entity_material = true;
    }

    real32 edit_material_button_width = row_width - (x - initial_x) - padding_right;
    bool32 edit_material_pressed = do_text_button(ui_manager, controller_state,
                                                  x, y,
                                                  edit_material_button_width, row_height,
                                                  button_style, default_text_style,
                                                  "Edit", editor_font_name_bold,
                                                  false,
                                                  "edit_material");

    
    x = initial_x;
    y += row_height;

    if (edit_material_pressed) {
        editor_state->editing_selected_entity_material = !editor_state->editing_selected_entity_material;
    }

    real32 edit_box_width = choose_material_button_width + padding_left + add_material_button_width;
    if (editor_state->editing_selected_entity_material) {
        draw_row_padding(ui_manager, controller_state, x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);

        UI_Text_Box_Style text_box_style = default_text_box_style;

        // MATERIAL NAME
        draw_row(ui_manager, controller_state, x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(game_state, ui_manager, x + padding_left, y, row_height,
                             "Material Name", editor_font_name_bold, default_text_style);
        do_text_box(ui_manager, controller_state,
                    x + right_column_offset, y,
                    edit_box_width, row_height,
                    &material->name, editor_font_name_bold,
                    text_box_style, default_text_style,
                    "material_name_text_box");

        y += row_height;
        draw_row_padding(ui_manager, controller_state, x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);

        // TEXTURE
        char *texture_name = "";
        if (material->texture_id >= 0) {
            Texture texture = get_texture(game_state, material->texture_id);
            texture_name = to_char_array(allocator, texture.name);
        }
        draw_row(ui_manager, controller_state, x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(game_state, ui_manager, x + padding_left, y, row_height,
                             "Texture", editor_font_name_bold, text_style);
        bool32 choose_texture_pressed = do_text_button(ui_manager, controller_state,
                                                       x + right_column_offset, y,
                                                       edit_box_width, row_height,
                                                       button_style, default_text_style,
                                                       texture_name, editor_font_name_bold, "choose_texture");
        if (!editor_state->open_window_flags && choose_texture_pressed) {
            editor_state->open_window_flags |= TEXTURE_LIBRARY_WINDOW;
        }

        y += row_height;
        draw_row_padding(ui_manager, controller_state, x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);

        // GLOSS
        draw_row(ui_manager, controller_state, x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        buf = string_format(allocator, buffer_size, "%f", material->gloss);
        draw_v_centered_text(game_state, ui_manager, x + padding_left, y, row_height,
                             "Gloss", editor_font_name_bold, text_style);
        material->gloss = do_slider(ui_manager, controller_state,
                                    x+right_column_offset, y,
                                    edit_box_width, row_height,
                                    buf, editor_font_name_bold,
                                    0.0f, 100.0f, material->gloss,
                                    default_slider_style, default_text_style,
                                    "edit_material_gloss_slider");
        y += row_height;
        draw_row_padding(ui_manager, controller_state, x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);

        // COLOR OVERRIDE
        draw_row(ui_manager, controller_state, x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        buf = string_format(allocator, buffer_size, "%f", material->gloss);
        draw_v_centered_text(game_state, ui_manager, x + padding_left, y, row_height,
                             "Color Override", editor_font_name_bold, text_style);
        do_color_button(ui_manager, controller_state,
                        x + right_column_offset, y,
                        row_height, row_height,
                        default_color_button_style,
                        material->color_override,
                        "material_color_override");
        y += row_height;
        draw_row_padding(ui_manager, controller_state, x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);

        // USE COLOR OVERRIDE
        draw_row(ui_manager, controller_state, x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(game_state, ui_manager, x + padding_left, y, row_height,
                             "Use Color Override", editor_font_name_bold, text_style);
        bool32 toggle_color_override_pressed = do_text_button(ui_manager, controller_state,
                                                              x + right_column_offset, y,
                                                              100.0f, row_height,
                                                              button_style, default_text_style,
                                                              material->use_color_override ? "true" : "false",
                                                              editor_font_name_bold,
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

    draw_row_padding(ui_manager, controller_state, x, &y, row_width, padding_y, row_color,
                     side_flags | SIDE_BOTTOM, row_index++);

    if (entity->type == ENTITY_POINT_LIGHT) {
        draw_row_padding(ui_manager, controller_state, x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);
        Point_Light_Entity *point_light = (Point_Light_Entity *) entity;

        // LIGHT COLOR
        draw_row(ui_manager, controller_state, x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(game_state, ui_manager, x + padding_left, y, row_height,
                             "Light Color", editor_font_name_bold, text_style);
        do_color_button(ui_manager, controller_state,
                        x + right_column_offset, y,
                        row_height, row_height,
                        default_color_button_style,
                        make_vec4(point_light->light_color, 1.0f),
                        "point_light_color");
        
        y += row_height;

        // LIGHT MIN DISTANCE
        draw_row_padding(ui_manager, controller_state, x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);
        draw_row(ui_manager, controller_state, x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        buf = string_format(allocator, buffer_size, "%f", point_light->d_min);
        draw_v_centered_text(game_state, ui_manager, x + padding_left, y, row_height,
                             "Falloff Start", editor_font_name_bold, text_style);
        point_light->d_min = do_slider(ui_manager, controller_state,
                                    x+right_column_offset, y,
                                    edit_box_width, row_height,
                                    buf, editor_font_name_bold,
                                    0.0f, 100.0f, point_light->d_min,
                                    default_slider_style, default_text_style,
                                    "edit_material_gloss_slider");
        y += row_height;
        draw_row_padding(ui_manager, controller_state, x, &y, row_width, padding_y, row_color,
                         side_flags, row_index++);

        // LIGHT MAX DISTANCE
        draw_row(ui_manager, controller_state, x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        buf = string_format(allocator, buffer_size, "%f", point_light->d_max);
        draw_v_centered_text(game_state, ui_manager, x + padding_left, y, row_height,
                             "Falloff Distance", editor_font_name_bold, text_style);
        point_light->d_max = do_slider(ui_manager, controller_state,
                                       x+right_column_offset, y,
                                       edit_box_width, row_height,
                                       buf, editor_font_name_bold,
                                       0.0f, 100.0f, point_light->d_max,
                                       default_slider_style, default_text_style,
                                       "edit_material_gloss_slider");
        y += row_height;
        draw_row_padding(ui_manager, controller_state, x, &y, row_width, padding_y, row_color,
                         side_flags | SIDE_BOTTOM, row_index++);
    }
}

void draw_editor_ui(Game_State *game_state, Controller_State *controller_state) {
    Editor_State *editor_state = &game_state->editor_state;
    UI_Manager *ui_manager = &game_state->ui_manager;
    Render_State *render_state = &game_state->render_state;

    real32 y = 0.0f;
    real32 button_gap = 1.0f;
    
    real32 button_height = 25.0f;
    char *button_font_name = editor_font_name_bold;
    // wireframe toggle
    real32 wireframe_button_width = 200.0f;
    bool32 toggle_show_wireframe_clicked = do_text_button(ui_manager, controller_state,
                                                          render_state->display_output.width - wireframe_button_width, y,
                                                          wireframe_button_width, button_height,
                                                          default_text_button_style, default_text_style,
                                                          editor_state->show_wireframe ? "Hide Wireframe" : "Show Wireframe",
                                                          button_font_name, "toggle_wireframe");
    if (toggle_show_wireframe_clicked) {
        editor_state->show_wireframe = !editor_state->show_wireframe;
    }
    y += button_height + button_gap;

    // transform mode toggle
    real32 toggle_global_button_width = 200.0f;
    bool32 toggle_global_clicked = do_text_button(ui_manager, controller_state,
                                                  render_state->display_output.width - toggle_global_button_width, y,
                                                  toggle_global_button_width, button_height,
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
    
    if (editor_state->selected_entity_index >= 0) {
        Entity *selected_entity = get_selected_entity(game_state);
        draw_entity_box(game_state, controller_state, selected_entity);

        if (editor_state->open_window_flags & MATERIAL_LIBRARY_WINDOW) {
            draw_material_library(game_state, controller_state, selected_entity);
        } else if (editor_state->open_window_flags & TEXTURE_LIBRARY_WINDOW) {
            Material *selected_material;
            bool32 material_exists = hash_table_find_pointer(game_state->material_table,
                                                             selected_entity->material_id,
                                                             &selected_material);
            assert(material_exists);
            draw_texture_library(game_state, controller_state, selected_material);
        } else if (editor_state->open_window_flags & MESH_LIBRARY_WINDOW) {
            draw_mesh_library(game_state, controller_state, selected_entity);
        }
    } else {
        reset_entity_editors(editor_state);
    }
}

int32 ray_intersects_mesh(Ray ray, Mesh mesh, Transform transform, real32 *t_result) {
    Mat4 object_to_world = get_model_matrix(transform);
    Mat4 world_to_object = inverse(object_to_world);

    // instead of transforming every vertex, we just transform the world-space ray to object-space by
    // multiplying the origin and the direction of the ray by the inverse of the entity's model matrix.
    // note that we must zero-out the w component of the ray direction when doing this multiplication so that
    // we ignore the translation of the model matrix.
    Vec3 object_space_ray_origin = truncate_v4_to_v3(world_to_object * make_vec4(ray.origin, 1.0f));
    Vec3 object_space_ray_direction = normalize(truncate_v4_to_v3(world_to_object *
                                                                  make_vec4(ray.direction, 0.0f)));
    Ray object_space_ray = { object_space_ray_origin, object_space_ray_direction };

    uint32 *indices = mesh.indices;

    // this might be very slow
    real32 t_min = FLT_MAX;
    bool32 hit = false;
    for (int32 i = 0; i < (int32) mesh.num_triangles; i++) {
        Vec3 triangle[3];
        triangle[0] = get_vertex_from_index(&mesh, indices[i * 3]);
        triangle[1] = get_vertex_from_index(&mesh, indices[i * 3 + 1]);
        triangle[2] = get_vertex_from_index(&mesh, indices[i * 3 + 2]);
        real32 t;

        // TODO: we might be able to pass t_min into this test to early-out before we check if a hit point
        //       is within the triangle, but after we've hit the plane
        if (ray_intersects_triangle(object_space_ray, triangle, &t)) {
            t_min = min(t, t_min);
            hit = true;
        }
    }

    if (hit) {
        // convert the t_min on the object space ray to a t_min on the world space ray
        Vec3 object_space_hit_point = object_space_ray_origin + object_space_ray_direction * t_min;
        Vec3 world_space_hit_point = truncate_v4_to_v3(object_to_world * make_vec4(object_space_hit_point, 1.0f));
        real32 world_space_t_min = dot(world_space_hit_point - ray.origin, ray.direction);
        *t_result = world_space_t_min;
    }

    return hit;
}

// TODO: optimize this by checking against AABB before checking against triangles
int32 pick_entity(Game_State *game_state, Ray cursor_ray, Entity *entity_result, int32 *index_result) {
    Editor_State *editor_state = &game_state->editor_state;

    Hash_Table<int32, Mesh> mesh_table = game_state->mesh_table;
    Normal_Entity *entities = game_state->entities;
    Point_Light_Entity *point_lights = game_state->point_lights;

    Entity *picked_entity = NULL;
    int32 entity_index = -1;

    real32 t_min = FLT_MAX;
    for (int32 i = 0; i < game_state->num_entities; i++) {
        real32 t;
        Normal_Entity *entity = &entities[i];
        Mesh mesh = hash_table_get(mesh_table, entity->mesh_id);
        if (ray_intersects_mesh(cursor_ray, mesh, entity->transform, &t) && (t < t_min)) {
            t_min = t;
            entity_index = i;
            picked_entity = (Entity *) entity;
        }
    }

    for (int32 i = 0; i < game_state->num_point_lights; i++) {
        real32 t;
        Point_Light_Entity *entity = &point_lights[i];
        Mesh mesh = hash_table_get(mesh_table, entity->mesh_id);
        if (ray_intersects_mesh(cursor_ray, mesh, entity->transform, &t) && (t < t_min)) {
            t_min = t;
            entity_index = i;
            picked_entity = (Entity *) entity;
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
    Mesh arrow_mesh = get_mesh(game_state, gizmo.arrow_mesh_id);

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
    Mesh ring_mesh = get_mesh(game_state, gizmo.ring_mesh_id);

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
