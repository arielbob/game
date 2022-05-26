#include "editor.h"
#include "level.h"

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

void draw_centered_text(real32 box_x, real32 box_y,
                        real32 row_width, real32 row_height,
                        char *text, int32 font_id, UI_Text_Style text_style) {
    using namespace Context;
    Font font = get_font(&editor_state->asset_manager, font_id);

    real32 adjusted_text_height = font.height_pixels - font.scale_for_pixel_height * (font.ascent + font.descent);
    real32 x_offset = 0.5f * row_width - 0.5f * get_width(font, text);
    real32 y_offset = 0.5f * (row_height + adjusted_text_height);
    do_text(ui_manager, box_x + x_offset, box_y + y_offset,
            text, font_id, text_style, "entity_properties_text");
}

void draw_v_centered_text(real32 box_x, real32 box_y,
                          real32 row_height,
                          char *text, int32 font_id, UI_Text_Style text_style) {
    using namespace Context;
    Font font = get_font(&editor_state->asset_manager, font_id);

    real32 adjusted_text_height = font.height_pixels - font.scale_for_pixel_height * (font.ascent + font.descent);
    real32 y_offset = 0.5f * (row_height + adjusted_text_height);
    do_text(ui_manager, box_x, box_y + y_offset,
            text, font_id, text_style, "entity_properties_text");
}

void draw_labeled_text(real32 x, real32 y,
                       real32 row_height,
                       int32 label_font_id, char *label,
                       int32 text_font_id, char *text,
                       UI_Text_Style text_style) {
    using namespace Context;
    real32 small_spacing = 20.0f;
    draw_v_centered_text(x, y, row_height,
                         label, label_font_id, text_style);
    draw_v_centered_text(x+small_spacing, y, row_height,
                         text, text_font_id, text_style);
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

void handle_color_picker(Editor_State *editor_state, UI_Color_Picker_Result result) {
    if (result.should_hide) {
        editor_state->color_picker_parent = {};
    }
}

void start_or_end_entity_change(Editor_State *editor_state,
                                UI_Manager *ui_manager, UI_id element_id,
                                Entity *entity) {
    if (is_newly_active(ui_manager, element_id)) {
        start_entity_change(editor_state, entity);
    } else if (is_newly_inactive(ui_manager, element_id)) {
        end_entity_change(editor_state, entity);
    }
}

void start_or_end_material_change(Editor_State *editor_state,
                                  UI_Manager *ui_manager, UI_id element_id,
                                  int32 material_id) {
    if (is_newly_active(ui_manager, element_id)) {
        start_material_change(editor_state, material_id);
    } else if (is_newly_inactive(ui_manager, element_id)) {
        end_material_change(editor_state, material_id);
    }
}

bool32 editor_add_mesh_press(Editor_State *editor_state, int32 entity_id) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;

    Marker m = begin_region();
    char *absolute_filename = (char *) region_push(PLATFORM_MAX_PATH);
    
    if (platform_open_file_dialog(absolute_filename, PLATFORM_MAX_PATH)) {
        char *mesh_name_buffer = (char *) region_push(MESH_NAME_MAX_SIZE);
        bool32 result = generate_mesh_name(asset_manager, mesh_name_buffer, MESH_NAME_MAX_SIZE);
        assert(result);
        String mesh_name = make_string(mesh_name_buffer);

        char *relative_filename = (char *) region_push(PLATFORM_MAX_PATH);
        platform_get_relative_path(absolute_filename, relative_filename, PLATFORM_MAX_PATH);
        String filename = make_string(relative_filename);

        do_add_mesh(editor_state, filename, mesh_name, entity_id);

        end_region(m);
        return true;
    }

    return false;
}

void editor_add_material_press(Editor_State *editor_state, int32 entity_id) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;

    Marker m = begin_region();
    
    char *material_name_buffer = (char *) region_push(MATERIAL_NAME_MAX_SIZE);
    bool32 result = generate_material_name(asset_manager, material_name_buffer, MATERIAL_NAME_MAX_SIZE);
    assert(result);

    String material_name = make_string(material_name_buffer);

    do_add_material(editor_state, material_name, entity_id);

    end_region(m);
}

bool32 editor_add_texture_press(Editor_State *editor_state, int32 material_id) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;

    Marker m = begin_region();
    char *absolute_filename = (char *) region_push(PLATFORM_MAX_PATH);
    
    if (platform_open_file_dialog(absolute_filename, PLATFORM_MAX_PATH)) {
        char *texture_name_buffer = (char *) region_push(TEXTURE_NAME_MAX_SIZE);
        bool32 result = generate_texture_name(asset_manager, texture_name_buffer, TEXTURE_NAME_MAX_SIZE);
        assert(result);
        String texture_name = make_string(texture_name_buffer);

        char *relative_filename = (char *) region_push(PLATFORM_MAX_PATH);
        platform_get_relative_path(absolute_filename, relative_filename, PLATFORM_MAX_PATH);
        String filename = make_string(relative_filename);

        do_add_texture(editor_state, filename, texture_name, material_id);

        end_region(m);
        return true;
    }

    return false;
}

void draw_texture_library(Editor_State *editor_state, UI_Manager *ui_manager, Controller_State *controller_state,
                           Render_State *render_state, int32 material_id) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;

    Material *material = get_material_pointer(asset_manager, material_id);

    push_layer(ui_manager);

    real32 padding_x = Editor_Constants::medium_padding_x;
    real32 padding_y = Editor_Constants::medium_padding_y;

    real32 x_gap = Editor_Constants::small_padding_x;
    real32 y_gap = Editor_Constants::small_padding_y;

    real32 small_row_height = Editor_Constants::small_row_height;

    int32 num_items_per_row = 5;
    real32 item_width = 100.0f;
    real32 item_height = 100.0f;

    real32 window_width = padding_x * 2 + x_gap * (num_items_per_row - 1) + num_items_per_row*item_width;

    real32 title_row_height = 50.0f;
    Vec4 title_row_color = make_vec4(0.05f, 0.2f, 0.5f, 1.0f);
    Vec4 row_color = Editor_Constants::row_color;
    
    char *row_id = "texture_library_row";
    int32 row_index = 0;

    int32 font_id;
    Font editor_font = get_font(asset_manager, Editor_Constants::editor_font_name, &font_id);

    real32 initial_x = render_state->display_output.width / 2.0f - window_width / 2.0f;
    real32 x = initial_x;
    real32 y = 80.0f;

    UI_Text_Style text_style = default_text_style;

    draw_row(x, y, window_width, title_row_height, title_row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_TOP | SIDE_BOTTOM, row_id, row_index++);
    draw_centered_text(x, y, window_width, title_row_height,
                       "Texture Library", font_id, text_style);
    y += title_row_height;

    real32 content_height = 500.0f;
    draw_row(x, y, window_width, content_height, row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_BOTTOM, row_id, row_index++);    

    real32 cancel_button_width = 100.0f;
    bool32 cancel_pressed = do_text_button(x + padding_x,
                                           y + content_height - small_row_height - padding_y - 1,
                                           cancel_button_width, small_row_height,
                                           default_text_button_cancel_style, default_text_style,
                                           "Cancel",
                                           font_id,
                                           "choose_texture_cancel");
    if (cancel_pressed) {
        editor_state->open_window_flags = 0;
    }

    Allocator *allocator = (Allocator *) &memory.frame_arena;

    x += padding_x;
    y += padding_y;

    UI_Image_Button_Style image_button_style = default_image_button_style;
    image_button_style.image_constraint_flags = (CONSTRAINT_FILL_BUTTON_WIDTH |
                                                 CONSTRAINT_FILL_BUTTON_HEIGHT |
                                                 CONSTRAINT_KEEP_IMAGE_PROPORTIONS);
                                                 

    Hash_Table<int32, Texture> *texture_table = &asset_manager->texture_table;
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
                                         entry->key, texture_name, font_id,
                                         button_id_string, i);

        if (pressed) picked_texture_id = entry->key;
        x += item_width + x_gap;
        if (x + item_width > initial_x + window_width) {
            x = initial_x + padding_x;
            y += item_height + y_gap;
        }
    }
    
    if (picked_texture_id >= 0) {
        if (material->texture_id < 0 ||
            picked_texture_id != material->texture_id) {
            start_material_change(editor_state, material_id);
            material->texture_id = picked_texture_id;
            end_material_change(editor_state, material_id);
        }

        editor_state->open_window_flags = 0;
    }

    pop_layer(ui_manager);
}

void draw_mesh_library(Editor_State *editor_state, UI_Manager *ui_manager, Controller_State *controller_state,
                       Render_State *render_state,
                       Entity *entity) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;

    push_layer(ui_manager);

    real32 padding_x = Editor_Constants::medium_padding_x;
    real32 padding_y = Editor_Constants::medium_padding_y;

    real32 small_row_height = Editor_Constants::small_row_height;

    real32 x_gap = Editor_Constants::small_padding_x;
    real32 y_gap = Editor_Constants::small_padding_y;

    int32 num_items_per_row = 5;
    real32 item_width = 100.0f;
    real32 item_height = 120.0f;

    real32 window_width = padding_x * 2 + x_gap * (num_items_per_row - 1) + num_items_per_row*item_width;

    real32 title_row_height = 50.0f;
    Vec4 title_row_color = make_vec4(0.05f, 0.2f, 0.5f, 1.0f);
    Vec4 row_color = Editor_Constants::row_color;

    int32 font_id;
    Font font = get_font(asset_manager, Editor_Constants::editor_font_name, &font_id);
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
                       "Mesh Library", font_id, text_style);
    y += title_row_height;

    real32 content_height = 500.0f;
    draw_row(x, y, window_width, content_height, row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_BOTTOM, row_id, row_index++);    

    real32 cancel_button_width = 100.0f;
    bool32 cancel_pressed = do_text_button(x + padding_x,
                                           y + content_height - small_row_height - padding_y - 1,
                                           cancel_button_width, small_row_height,
                                           default_text_button_cancel_style, default_text_style,
                                           "Cancel",
                                           font_id,
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
    real32 filter_button_height = small_row_height;

    real32 button_gap = 5.0f;
    bool32 all_filter_pressed = do_text_button(x, y,
                                               filter_button_width, filter_button_height,
                                               filter_button_style, default_text_style,
                                               "All",
                                               font_id,
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
                                                 font_id,
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
                                               font_id,
                                               editor_state->mesh_library_filter == Mesh_Type::PRIMITIVE,
                                               "mesh_filter_primitives");
    if (primitives_pressed) editor_state->mesh_library_filter = Mesh_Type::PRIMITIVE;
    x += filter_button_width;

    y += filter_button_height + x_gap;
                   
    x = initial_x + padding_x;
    UI_Image_Button_Style image_button_style = default_image_button_style;
    image_button_style.image_constraint_flags = CONSTRAINT_FILL_BUTTON_WIDTH | CONSTRAINT_KEEP_IMAGE_PROPORTIONS;

    //Hash_Table<int32, Mesh> *mesh_table = &game_state->current_level.mesh_table;
    //Mesh_Type picked_mesh_type = Mesh_Type::NONE;
    int32 picked_mesh_id = -1;

    FOR_ENTRY_POINTERS(int, Mesh, asset_manager->mesh_table) {
        Mesh *mesh = &entry->value;
        if (editor_state->mesh_library_filter != Mesh_Type::NONE &&
            mesh->type != editor_state->mesh_library_filter) continue;

        char *mesh_name = to_char_array(allocator, mesh->name);
        UI_Text_Button_Style text_button_style = default_text_button_style;

        if (mesh->type == Mesh_Type::PRIMITIVE) {
            text_button_style = primitive_item_style;
        } else if (mesh->type == Mesh_Type::LEVEL) {
            text_button_style = level_item_style;
        } else {
            continue;
        }

        bool32 pressed = do_text_button(x, y,
                                        item_width, item_height,
                                        text_button_style, default_text_style,
                                        mesh_name,
                                        font_id,
                                        "mesh_library_level_item", entry->key);

        if (pressed) {
            picked_mesh_id = entry->key;
        }

        x += item_width + x_gap;
        if (x + item_width > initial_x + window_width) {
            x = initial_x + padding_x;
            y += item_height + y_gap;
        }
    }

    if (picked_mesh_id >= 0) {
        start_entity_change(editor_state, entity);
        set_mesh(asset_manager, entity, picked_mesh_id);
        end_entity_change(editor_state, entity);

        editor_state->open_window_flags = 0;
    }

    pop_layer(ui_manager);
}

void draw_material_library(Editor_State *editor_state, UI_Manager *ui_manager, Controller_State *controller_state,
                           Render_State *render_state,
                           Entity *entity) {
    push_layer(ui_manager);

    Asset_Manager *asset_manager = &editor_state->asset_manager;
    
    real32 padding_x = Editor_Constants::small_padding_x;
    real32 padding_y = Editor_Constants::small_padding_y;

    real32 small_row_height = Editor_Constants::small_row_height;

    real32 x_gap = padding_x;
    real32 y_gap = padding_y;

    int32 num_items_per_row = 1;
    real32 item_width = 500.0f;
    real32 item_height = 20.0f;

    real32 window_width = padding_x * 2 + x_gap * (num_items_per_row - 1) + num_items_per_row*item_width;

    real32 title_row_height = 50.0f;
    Vec4 title_row_color = make_vec4(0.05f, 0.2f, 0.5f, 1.0f);
    Vec4 row_color = Editor_Constants::row_color;

    char *row_id = "material_library_row";
    int32 row_index = 0;

    real32 initial_x = render_state->display_output.width / 2.0f - window_width / 2.0f;
    real32 x = initial_x;
    real32 y = 80.0f;

    UI_Text_Style text_style = default_text_style;

    int32 font_id;
    Font editor_font = get_font(asset_manager, Editor_Constants::editor_font_name, &font_id);

    draw_row(x, y, window_width, title_row_height, title_row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_TOP | SIDE_BOTTOM, row_id, row_index++);
    draw_centered_text(x, y, window_width, title_row_height,
                       "Material Library", font_id, text_style);
    y += title_row_height;

    real32 content_height = 500.0f;
    draw_row(x, y, window_width, content_height, row_color,
             SIDE_LEFT | SIDE_RIGHT | SIDE_BOTTOM, row_id, row_index++);    

    real32 cancel_button_width = 100.0f;
    bool32 cancel_pressed = do_text_button(x + padding_x,
                                           y + content_height - small_row_height - padding_y - 1,
                                           cancel_button_width, small_row_height,
                                           default_text_button_cancel_style, default_text_style,
                                           "Cancel",
                                           font_id,
                                           "choose_material_cancel");
    if (cancel_pressed) {
        editor_state->open_window_flags = 0;
    }
    
    Allocator *allocator = (Allocator *) &memory.frame_arena;

    x += padding_x;
    y += padding_y;

    Hash_Table<int32, Material> *material_table = &asset_manager->material_table;
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
                                        font_id,
                                        button_id_string, i);

        if (pressed) picked_material_id = entry->key;
        x += item_width + x_gap;
        if (x + item_width > initial_x + window_width) {
            x = initial_x + padding_x;
            y += item_height + y_gap;
        }
    }
    
    if (picked_material_id >= 0) {
        start_entity_change(editor_state, entity);
        set_material(entity, picked_material_id);
        end_entity_change(editor_state, entity);
        editor_state->open_window_flags = 0;
    }

    pop_layer(ui_manager);
};

void draw_entity_box(Editor_State *editor_state, UI_Manager *ui_manager, Controller_State *controller_state,
                     Entity *entity) {
    int32 row_index = 0;

    Asset_Manager *asset_manager = &editor_state->asset_manager;

    real32 box_x = 5.0f;
    real32 box_y = 50.0f;

    real32 box_padding_x = 10.0f;
    real32 box_padding_y = 10.0f;

    Allocator *allocator = (Allocator *) &memory.frame_arena;

    Editor_Level *level = &editor_state->level;
    //Mesh *mesh = get_mesh_pointer(game_state, level, entity->mesh_type, entity->mesh_id);
    // bool32 material_exists = hash_table_find_pointer(game_state->current_level.material_table,
    //                                                  entity->material_id,
    //                                                  &material);
    // assert(material_exists);
    Transform transform = entity->transform;

    UI_Text_Button_Style button_style = default_text_button_style;
    
    UI_Text_Style text_style = default_text_style;
    
    real32 padding_x = Editor_Constants::small_padding_x;
    real32 padding_y = Editor_Constants::small_padding_y;
    real32 x_nested_offset = Editor_Constants::x_nested_offset;
    real32 right_column_offset = padding_x + 130.0f;
    real32 small_spacing = 20.0f;

    real32 initial_row_height = 22.0f;
    real32 row_height = initial_row_height;
    real32 small_row_height = Editor_Constants::small_row_height;
    //real32 small_row_height = SMALL_ROW_HEIGHT;
    real32 row_width = 400.0f;

    Vec4 row_color = Editor_Constants::row_color;
    Vec4 darkened_row_color = Editor_Constants::darkened_row_color;
    Vec4 title_row_color = make_vec4(0.05f, 0.2f, 0.5f, 1.0f);

    real32 initial_x = box_x;
    real32 x = box_x;
    real32 y = box_y;

    real32 title_row_height = 30.0f;
    uint32 side_flags = SIDE_LEFT | SIDE_RIGHT;

    char *row_id = "entity_properties_title";

    int32 font_id;
    Font editor_font = get_font(asset_manager, Editor_Constants::editor_font_name, &font_id);

    draw_row(x, y, row_width, title_row_height, title_row_color,
             side_flags | SIDE_TOP, row_id, row_index++);
    draw_centered_text(x, y, row_width, title_row_height,
                       "Entity Properties", font_id, text_style);
    y += title_row_height;
    draw_row_line(x, &y, row_width);
        
    int32 entity_id = entity->id;

    char *buf;
    int32 buffer_size = 16;
    
    // POSITION
    draw_row(x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    draw_v_centered_text(x+padding_x, y, small_row_height,
                         "Position", font_id, text_style);
    // x
    //buf = string_format(allocator, buffer_size, "%f", transform.position.x);
    x += right_column_offset;
    real32 transform_button_width = 80.0f;

    UI_id x_slider, y_slider, z_slider;
    real32 new_x = do_slider(x, y,
                             transform_button_width, small_row_height,
                             font_id,
                             transform.position.x,
                             default_slider_style, default_text_style,
                             "x", entity_id,
                             &x_slider);
    x += transform_button_width + 5.0f;

    real32 new_y = do_slider(x, y,
                             transform_button_width, small_row_height,
                             font_id,
                             transform.position.y,
                             default_slider_style, default_text_style,
                             "y", entity_id,
                             &y_slider);
    x += transform_button_width + 5.0f;

    real32 new_z = do_slider(x, y,
                             transform_button_width, small_row_height,
                             font_id,
                             transform.position.z,
                             default_slider_style, default_text_style,
                             "z", entity_id,
                             &z_slider);
    x += transform_button_width + 5.0f;

    update_entity_position(asset_manager, entity, make_vec3(new_x, new_y, new_z));
    start_or_end_entity_change(editor_state, ui_manager, x_slider, entity);
    start_or_end_entity_change(editor_state, ui_manager, y_slider, entity);
    start_or_end_entity_change(editor_state, ui_manager, z_slider, entity);

    y += small_row_height;
    x = initial_x;

    draw_row_line(x, &y, row_width);

    // ROTATION
    draw_row(x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    draw_v_centered_text(x+padding_x, y, small_row_height,
                         "Rotation", font_id, text_style);
    // w
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.w);
    draw_labeled_text(x + right_column_offset, y, small_row_height,
                      font_id, "w", font_id, buf, text_style);
    y += small_row_height;
    // x
    draw_row(x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.v.x);
    draw_labeled_text(x + right_column_offset, y, small_row_height,
                      font_id, "x", font_id, buf, text_style);
    y += small_row_height;
    // y
    draw_row(x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.v.y);
    draw_labeled_text(x + right_column_offset, y, small_row_height,
                      font_id, "y", font_id, buf, text_style);
    y += small_row_height;
    // z
    draw_row(x, y, row_width, small_row_height, row_color, side_flags,
             row_id, row_index++);
    buf = string_format(allocator, buffer_size, "%f", transform.rotation.v.z);
    draw_labeled_text(x + right_column_offset, y, small_row_height,
                      font_id, "z", font_id, buf, text_style);
    y += small_row_height;

    draw_row_line(x, &y, row_width);

    // SCALE
    draw_row(x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    draw_v_centered_text(x+padding_x, y, small_row_height,
                         "Scale", font_id, text_style);
    x += right_column_offset;

    UI_id scale_x_slider, scale_y_slider, scale_z_slider;
    real32 new_scale_x = do_slider(x, y, transform_button_width, small_row_height,
                                   font_id,
                                   transform.scale.x,
                                   default_slider_style, default_text_style,
                                   "scale_x", entity_id,
                                   &scale_x_slider);
    x += transform_button_width + 5.0f;

    real32 new_scale_y = do_slider(x, y, transform_button_width, small_row_height,
                                   font_id,
                                   transform.scale.y,
                                   default_slider_style, default_text_style,
                                   "scale_y", entity_id,
                                   &scale_y_slider);
    x += transform_button_width + 5.0f;

    real32 new_scale_z = do_slider(x, y, transform_button_width, small_row_height,
                                   font_id,
                                   transform.scale.z,
                                   default_slider_style, default_text_style,
                                   "scale_z", entity_id,
                                   &scale_z_slider);
    x += transform_button_width + 5.0f;

    update_entity_scale(asset_manager, entity, make_vec3(new_scale_x, new_scale_y, new_scale_z));
    start_or_end_entity_change(editor_state, ui_manager, scale_x_slider, entity);
    start_or_end_entity_change(editor_state, ui_manager, scale_y_slider, entity);
    start_or_end_entity_change(editor_state, ui_manager, scale_z_slider, entity);

    y += small_row_height;
    x = initial_x;

    draw_row_line(x, &y, row_width);

    real32 wide_button_width = 200.0f;
    real32 small_button_width = 30.0f;
    real32 edit_box_width = wide_button_width - small_button_width;
    
    if (entity->type == ENTITY_NORMAL) {
        Normal_Entity *normal_entity = (Normal_Entity *) entity;

        Mesh *mesh = get_mesh_pointer(asset_manager, normal_entity->mesh_id);
        
        row_id = "mesh_properties_line";
    
        // MESH PROPERTIES
        real32 choose_mesh_button_width = edit_box_width - small_button_width;

        char *mesh_name = to_char_array(allocator, mesh->name);
        draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags,
                         row_id, row_index++);
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        char *mesh_label_string = "Mesh";
        if (mesh->type == Mesh_Type::PRIMITIVE) mesh_label_string = "Mesh (primitive)";
        draw_v_centered_text(x + padding_x, y, row_height,
                             mesh_label_string, font_id, text_style);

        x += right_column_offset;
        UI_Text_Button_Style choose_mesh_button_style = button_style;
        choose_mesh_button_style.corner_flags = CORNER_TOP_LEFT | CORNER_BOTTOM_LEFT;
        bool32 choose_mesh_pressed = do_text_button(x, y, choose_mesh_button_width, row_height,
                                                    choose_mesh_button_style, default_text_style,
                                                    to_char_array(allocator, mesh->name),
                                                    font_id, "choose_mesh");

        if (!editor_state->open_window_flags && choose_mesh_pressed) {
            editor_state->open_window_flags |= MESH_LIBRARY_WINDOW;
        }
        x += choose_mesh_button_width;

        UI_Text_Button_Style delete_mesh_button_style = default_text_button_cancel_style;
        delete_mesh_button_style.corner_flags = CORNER_TOP_RIGHT | CORNER_BOTTOM_RIGHT;
        bool32 delete_mesh_pressed = do_text_button(x, y, small_button_width, row_height,
                                                    delete_mesh_button_style, default_text_style,
                                                    "-", font_id,
                                                    mesh->type == Mesh_Type::PRIMITIVE,
                                                    "delete_mesh");

        if (delete_mesh_pressed) {
            assert(mesh->type == Mesh_Type::LEVEL);
            do_delete_mesh(editor_state, normal_entity->mesh_id);
        }

        x += small_button_width + padding_x;

        bool32 add_mesh_pressed = do_text_button(x, y,
                                                 small_button_width, row_height,
                                                 button_style, default_text_style,
                                                 "+", font_id,
                                                 "add_mesh");
        x += small_button_width + padding_x;

        bool32 mesh_added = false;
        if (add_mesh_pressed) {
            mesh_added = editor_add_mesh_press(editor_state, normal_entity->id);
            if (mesh_added) {
                editor_state->editing_selected_entity_mesh = true;
                mesh = get_mesh_pointer(asset_manager, normal_entity->mesh_id);
            }
        }

        real32 edit_mesh_button_width = row_width - (x - initial_x) - padding_x;
        bool32 edit_mesh_pressed = do_text_button(x, y,
                                                  edit_mesh_button_width, row_height,
                                                  button_style, default_text_style,
                                                  "Edit", font_id,
                                                  mesh->type == Mesh_Type::PRIMITIVE,
                                                  "edit_mesh");

        if (edit_mesh_pressed) {
            editor_state->editing_selected_entity_mesh = !editor_state->editing_selected_entity_mesh;
        }

        y += row_height;
        x = initial_x;
        if (mesh->type == Mesh_Type::LEVEL && editor_state->editing_selected_entity_mesh) {
            draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags,
                             row_id, row_index++);

            row_color = Editor_Constants::darkened_row_color;
            draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags,
                             row_id, row_index++);
            UI_Text_Box_Style text_box_style = default_text_box_style;

            draw_row(x, y, row_width, row_height, row_color, side_flags,
                     row_id, row_index++);
            draw_v_centered_text(x + padding_x + Editor_Constants::x_nested_offset, y, row_height,
                                 "Mesh Name", font_id, default_text_style);

            UI_Text_Box_Result result = do_text_box(x + right_column_offset, y,
                                                    edit_box_width, row_height,
                                                    mesh->name, MESH_NAME_MAX_SIZE,
                                                    font_id,
                                                    text_box_style, default_text_style,
                                                    mesh_added,
                                                    "mesh_name_text_box", normal_entity->mesh_id);
            if (result.submitted) {
                String new_name = make_string(result.buffer);
                if (is_empty(new_name)) {
                    add_message(Context::message_manager, make_string("Mesh name cannot be empty!"));
                } else if (string_contains(new_name,
                                           Editor_Constants::disallowed_chars,
                                           Editor_Constants::num_disallowed_chars)) {
                    add_message(Context::message_manager, make_string("Mesh name cannot contain {, }, or double quotes!"));
                } else if (!string_equals(mesh->name, new_name)) {
                    if (!mesh_name_exists(asset_manager, new_name)) {
                        do_modify_mesh_name(editor_state, normal_entity->mesh_id, new_name);
                    } else {
                        add_message(Context::message_manager, make_string("Mesh name already exists!"));
                    }
                }
            }

            y += row_height;

            draw_row_padding(x, &y, row_width, padding_y, row_color,
                             side_flags | SIDE_BOTTOM,
                             row_id, row_index++);
            row_color = Editor_Constants::row_color;
        } else {
            draw_row_padding(x, &y, row_width, padding_y, row_color,
                             side_flags | SIDE_BOTTOM,
                             row_id, row_index++);
        }
        

        x = initial_x;

        // COLLIDER PROPERTIES
        draw_row(x, y, row_width, row_height, row_color, side_flags | SIDE_BOTTOM,
                 row_id, row_index++);
        draw_v_centered_text(x + padding_x, y, row_height,
                             "Collider", font_id, default_text_style);
        x += right_column_offset;

        char *collider_type_text = get_collider_type_string(normal_entity->collider.type);
        draw_v_centered_text(x, y, row_height,
                             collider_type_text, font_id, default_text_style);
        y += row_height + 1;
        x = initial_x;

        // WALKABLE PROPERTY
        draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags,
                         row_id, row_index++);
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(x + padding_x, y, row_height,
                             "Is Walkable", font_id, default_text_style);
        x += right_column_offset;

        char *is_walkable_text = normal_entity->is_walkable ? "true" : "false";
        bool32 toggle_is_walkable_pressed = do_text_button(x, y, 100.0f, row_height,
                                                           button_style, default_text_style,
                                                           is_walkable_text,
                                                           font_id,
                                                           false,
                                                           "toggle_is_walkable");
        y += row_height;
        x = initial_x;

        draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags | SIDE_BOTTOM,
                         row_id, row_index++);
        
        if (toggle_is_walkable_pressed) {
            start_entity_change(editor_state, (Entity *) normal_entity);
            normal_entity->is_walkable = !normal_entity->is_walkable;
            end_entity_change(editor_state, (Entity *) normal_entity);
        }

            
        // MATERIAL
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags, row_id, row_index++);

        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(x+padding_x, y, row_height,
                             "Material", font_id, text_style);

        side_flags = SIDE_LEFT | SIDE_RIGHT;

        x += right_column_offset;
        Material *material = NULL;
        char *material_name = "";
        bool32 has_material = normal_entity->material_id >= 0;
        if (has_material) {
            material = get_material_pointer(asset_manager, normal_entity->material_id);
            material_name = to_char_array(allocator, material->name);
        }

        real32 choose_material_button_width = edit_box_width - small_button_width;
        UI_Text_Button_Style choose_material_button_style = button_style;
        
        if (!has_material) {
            choose_material_button_width += small_button_width;
        } else {
            choose_material_button_style.corner_flags = CORNER_TOP_LEFT | CORNER_BOTTOM_LEFT;
        }
        bool32 choose_material_pressed = do_text_button(x, y, choose_material_button_width, row_height,
                                                        choose_material_button_style, default_text_style,
                                                        material_name,
                                                        font_id, "choose_material");

        if (!editor_state->open_window_flags && choose_material_pressed) {
            editor_state->open_window_flags |= MATERIAL_LIBRARY_WINDOW;
        }
        x += choose_material_button_width;

        if (has_material) {
            UI_Text_Button_Style delete_material_button_style = default_text_button_cancel_style;
            delete_material_button_style.corner_flags = CORNER_TOP_RIGHT | CORNER_BOTTOM_RIGHT;
            bool32 delete_material_pressed = do_text_button(x, y, small_button_width, row_height,
                                                            delete_material_button_style, default_text_style,
                                                            "-", font_id,
                                                            "delete_material");

            if (delete_material_pressed) {
                do_delete_material(editor_state, normal_entity->material_id);
            }

            x += small_button_width;
        }

        x += padding_x;

        real32 add_material_button_width = small_button_width;
        bool32 add_material_pressed = do_text_button(x, y,
                                                     add_material_button_width, row_height,
                                                     button_style, default_text_style,
                                                     "+", font_id, "add_material");
        x += add_material_button_width + padding_x;

        if (add_material_pressed) {
            editor_add_material_press(editor_state, entity_id);
        }

        real32 edit_material_button_width = row_width - (x - initial_x) - padding_x;
        bool32 edit_material_pressed = do_text_button(x, y,
                                                      edit_material_button_width, row_height,
                                                      button_style, default_text_style,
                                                      "Edit", font_id,
                                                      !has_material,
                                                      "edit_material");
    
        x = initial_x;
        y += row_height;

        if (edit_material_pressed) {
            editor_state->editing_selected_entity_material = !editor_state->editing_selected_entity_material;
        }

        if (has_material && editor_state->editing_selected_entity_material) {
            draw_row_padding(x, &y, row_width, padding_y, row_color,
                             side_flags, row_id, row_index++);
            row_color = Editor_Constants::darkened_row_color;

            draw_row_padding(x, &y, row_width, padding_y, row_color,
                             side_flags, row_id, row_index++);

            UI_Text_Box_Style text_box_style = default_text_box_style;

            real32 label_x = x + padding_x + x_nested_offset;

            // MATERIAL NAME
            draw_row(x, y, row_width, row_height, row_color, side_flags,
                     row_id, row_index++);
            draw_v_centered_text(label_x, y, row_height,
                                 "Material Name", font_id, default_text_style);
            // NOTE: we pass in normal_entity->material_id as the text button's index, because if the material id
            //       changes, for example, from switching materials or creating a new material, AND the name
            //       editing panel is still open, then we want the state to be reset. since the material_id's are
            //       always different from each other (since they're created from an incrementing int), the text
            //       box state will automatically be regenerated if the entity's material changes.
            UI_Text_Box_Result material_name_text_box_result = do_text_box(x + right_column_offset, y,
                                                                           edit_box_width, row_height,
                                                                           material->name, MATERIAL_NAME_MAX_SIZE,
                                                                           font_id,
                                                                           text_box_style, default_text_style,
                                                                           false,
                                                                           "material_name_text_box",
                                                                           normal_entity->material_id);
            if (material_name_text_box_result.submitted) {
                String new_name = make_string(material_name_text_box_result.buffer);
                if (is_empty(new_name)) {
                    add_message(Context::message_manager, make_string("Material name cannot be empty!"));
                } else if (string_contains(new_name,
                                           Editor_Constants::disallowed_chars,
                                           Editor_Constants::num_disallowed_chars)) {
                    add_message(Context::message_manager, make_string("Material name cannot contain {, }, or double quotes!"));
                } else if (!string_equals(material->name, new_name)) {
                    if (!material_name_exists(asset_manager, new_name)) {
                        int32 material_id = normal_entity->material_id;
                        start_material_change(editor_state, material_id);
                        replace_with_copy(asset_manager->allocator_pointer, &material->name, new_name);
                        end_material_change(editor_state, material_id);
                    } else {
                        add_message(Context::message_manager, make_string("Material name already exists!"));
                    }
                }
            }

#if 0
            String_Buffer text_result = do_text_box(x + right_column_offset, y,
                                                    edit_box_width, row_height,
                                                    &material->name, font_id,
                                                    text_box_style, default_text_style,
                                                    "material_name_text_box");
#endif

            y += row_height;
            draw_row_padding(x, &y, row_width, padding_y, row_color,
                             side_flags, row_id, row_index++);

            // TEXTURE
            char *texture_name = "";
            if (material->texture_id >= 0) {
                Texture texture = get_texture(asset_manager, material->texture_id);
                texture_name = to_char_array(allocator, texture.name);
            }
            draw_row(x, y, row_width, row_height, row_color, side_flags,
                     row_id, row_index++);
            draw_v_centered_text(label_x, y, row_height,
                                 "Texture", font_id, text_style);

            x += right_column_offset;
            bool32 has_texture = material->texture_id >= 0;

            real32 choose_texture_button_width = edit_box_width;
            if (has_texture) choose_texture_button_width -= small_button_width;

            bool32 choose_texture_pressed = do_text_button(x, y,
                                                           choose_texture_button_width, row_height,
                                                           button_style, default_text_style,
                                                           texture_name, font_id, "choose_texture");
            if (!editor_state->open_window_flags && choose_texture_pressed) {
                editor_state->open_window_flags |= TEXTURE_LIBRARY_WINDOW;
            }
            x += choose_texture_button_width;

            if (has_texture) {
                Texture *texture = get_texture_pointer(asset_manager, material->texture_id);
                bool32 delete_texture_pressed = do_text_button(x, y,
                                                               small_button_width, row_height,
                                                               default_text_button_cancel_style, default_text_style,
                                                               "-", font_id, "delete_texture");
                if (delete_texture_pressed) {
                    do_delete_texture(editor_state, material->texture_id);
                }

                x += small_button_width;
            }
            

            x += padding_x;

            bool32 add_texture_pressed = do_text_button(x, y,
                                                        small_button_width, row_height,
                                                        button_style, default_text_style,
                                                        "+", font_id, "add_texture");
            if (add_texture_pressed) {
                editor_add_texture_press(editor_state, normal_entity->material_id);
            }

            x += small_button_width + padding_x;
            real32 edit_texture_button_width = row_width - (x - initial_x) - padding_x;
            bool32 edit_texture_pressed = do_text_button(x, y,
                                                         edit_texture_button_width, row_height,
                                                         button_style, default_text_style,
                                                         "Edit", font_id,
                                                         !has_texture,
                                                         "edit_texture");
    
            x = initial_x;
            y += row_height;

            if (edit_texture_pressed) {
                editor_state->editing_selected_entity_texture = !editor_state->editing_selected_entity_texture;
            }

            if (has_texture && editor_state->editing_selected_entity_texture) {
                Texture *texture = get_texture_pointer(asset_manager, material->texture_id);
                draw_row_padding(x, &y, row_width, padding_y, row_color,
                                 side_flags, row_id, row_index++);

                // TEXTURE NAME
                draw_row(x, y, row_width, row_height, darkened_row_color, side_flags,
                         row_id, row_index++);
                draw_v_centered_text(label_x + x_nested_offset, y, row_height,
                                     "Texture Name", font_id, default_text_style);
                UI_Text_Box_Result result = do_text_box(x + right_column_offset, y, edit_box_width, row_height,
                                                        texture->name, TEXTURE_NAME_MAX_SIZE,
                                                        font_id,
                                                        text_box_style, default_text_style,
                                                        false,
                                                        "texture_name_text_box", material->texture_id);

                if (result.submitted) {
                    String new_name = make_string(result.buffer);
                    if (is_empty(new_name)) {
                        add_message(Context::message_manager, make_string("Texture name cannot be empty!"));
                    } else if (string_contains(new_name,
                                               Editor_Constants::disallowed_chars,
                                               Editor_Constants::num_disallowed_chars)) {
                        add_message(Context::message_manager, make_string("Texture name cannot contain {, }, or double quotes!"));
                    } else if (!string_equals(texture->name, new_name)) {
                        if (!texture_name_exists(asset_manager, new_name)) {
                            start_texture_change(editor_state, material->texture_id);
                            replace_with_copy(asset_manager->allocator_pointer, &texture->name, new_name);
                            end_texture_change(editor_state, material->texture_id);
                        } else {
                            add_message(Context::message_manager, make_string("Texture name already exists!"));
                        }
                    }
                }

                y += row_height;
            }

            x = initial_x;
            draw_row_padding(x, &y, row_width, padding_y, row_color,
                             side_flags, row_id, row_index++);

            // GLOSS
            draw_row(x, y, row_width, row_height, row_color, side_flags,
                     row_id, row_index++);
            draw_v_centered_text(label_x, y, row_height,
                                 "Gloss", font_id, text_style);
            UI_id gloss_slider;
            material->gloss = do_slider(x+right_column_offset, y,
                                        edit_box_width, row_height,
                                        font_id,
                                        0.0f, 100.0f, material->gloss,
                                        default_slider_style, default_text_style,
                                        "edit_material_gloss_slider", normal_entity->material_id,
                                        &gloss_slider);
            start_or_end_material_change(editor_state, ui_manager, gloss_slider, normal_entity->material_id);

            y += row_height;
            draw_row_padding(x, &y, row_width, padding_y, row_color,
                             side_flags, row_id, row_index++);

            // COLOR OVERRIDE
            draw_row(x, y, row_width, row_height, row_color, side_flags,
                     row_id, row_index++);
            buf = string_format(allocator, buffer_size, "%f", material->gloss);
            draw_v_centered_text(label_x, y, row_height,
                                 "Color Override", font_id, text_style);
            UI_id color_override_button_id;
            bool32 color_override_pressed = do_color_button(x + right_column_offset, y,
                                                            row_height, row_height,
                                                            default_color_button_style,
                                                            material->color_override,
                                                            "material_color_override_button", 0,
                                                            &color_override_button_id);
            if (ui_id_equals(editor_state->color_picker_parent, color_override_button_id)) {
                push_layer(ui_manager);
                UI_Color_Picker_Result result = do_color_picker(x + right_column_offset, y + row_height,
                                                                Editor_Constants::color_picker_style,
                                                                material->color_override,
                                                                "editor_color_picker", entity_id);
                pop_layer(ui_manager);
                handle_color_picker(editor_state, result);

                int32 material_id = normal_entity->material_id;
                if (result.started) {
                    start_material_change(editor_state, material_id);
                } else if (result.submitted) {
                    material->color_override = result.color;
                    end_material_change(editor_state, material_id);
                } else {
                    material->color_override = result.color;
                }
            } else if (color_override_pressed) {
                editor_state->color_picker_parent = color_override_button_id;
            }

            y += row_height;
            draw_row_padding(x, &y, row_width, padding_y, row_color,
                             side_flags, row_id, row_index++);

            // USE COLOR OVERRIDE
            draw_row(x, y, row_width, row_height, row_color, side_flags,
                     row_id, row_index++);
            draw_v_centered_text(label_x, y, row_height,
                                 "Use Color Override", font_id, text_style);
            bool32 toggle_color_override_pressed = do_text_button(x + right_column_offset, y,
                                                                  100.0f, row_height,
                                                                  button_style, default_text_style,
                                                                  ((material->texture_id < 0) || material->use_color_override) ? "true" : "false",
                                                                  font_id,
                                                                  material->texture_id < 0,
                                                                  "material_toggle_use_color_override");
            y += row_height;
        
            if (toggle_color_override_pressed) {
                start_material_change(editor_state, normal_entity->material_id);
                material->use_color_override = !material->use_color_override;
                if (material->texture_id < 0 && !material->use_color_override) {
                    material->use_color_override = true;
                }
                end_material_change(editor_state, normal_entity->material_id);
            }
        }
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags | SIDE_BOTTOM, row_id, row_index++);

        row_color = Editor_Constants::row_color;
    }
    
    if (entity->type == ENTITY_POINT_LIGHT) {
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags, row_id, row_index++);
        Point_Light_Entity *point_light = (Point_Light_Entity *) entity;

        // LIGHT COLOR
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(x + padding_x, y, row_height,
                             "Light Color", font_id, text_style);
        UI_id point_light_color_button_id;
        bool32 point_light_color_pressed = do_color_button(x + right_column_offset, y,
                                                           row_height, row_height,
                                                           default_color_button_style,
                                                           make_vec4(point_light->light_color, 1.0f),
                                                           "point_light_color", 0,
                                                           &point_light_color_button_id);

        if (ui_id_equals(editor_state->color_picker_parent, point_light_color_button_id)) {
            push_layer(ui_manager);
            UI_Color_Picker_Result result = do_color_picker(x + right_column_offset, y + row_height,
                                                            Editor_Constants::color_picker_style,
                                                            make_vec4(point_light->light_color, 1.0f),
                                                            "editor_color_picker", entity_id);
            pop_layer(ui_manager);
            handle_color_picker(editor_state, result);

            if (result.started) {
                start_entity_change(editor_state, entity);
            } else if (result.submitted) {
                point_light->light_color = truncate_v4_to_v3(result.color);
                end_entity_change(editor_state, entity);
            } else {
                point_light->light_color = truncate_v4_to_v3(result.color);
            }
        } else if (point_light_color_pressed) {
            editor_state->color_picker_parent = point_light_color_button_id;
        }

        y += row_height;

        // LIGHT MIN DISTANCE
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags, row_id, row_index++);
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(x + padding_x, y, row_height,
                             "Falloff Start", font_id, text_style);
        UI_id falloff_start_slider;
        point_light->falloff_start = do_slider(x+right_column_offset, y,
                                               edit_box_width, row_height,
                                               font_id,
                                               0.0f, 100.0f, point_light->falloff_start,
                                               default_slider_style, default_text_style,
                                               "edit_point_light_falloff_start_slider", entity_id,
                                               &falloff_start_slider);
        y += row_height;
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags, row_id, row_index++);

        // LIGHT MAX DISTANCE
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(x + padding_x, y, row_height,
                             "Falloff Distance", font_id, text_style);
        UI_id falloff_end_slider;
        point_light->falloff_end = do_slider(x+right_column_offset, y,
                                             edit_box_width, row_height,
                                             font_id,
                                             0.0f, 100.0f, point_light->falloff_end,
                                             default_slider_style, default_text_style,
                                             "edit_point_light_falloff_end_slider", entity_id,
                                             &falloff_end_slider);
        y += row_height;
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags | SIDE_BOTTOM, row_id, row_index++);

        start_or_end_entity_change(editor_state, ui_manager, falloff_start_slider, entity);
        start_or_end_entity_change(editor_state, ui_manager, falloff_end_slider, entity);
    }

    // DELETE ENTITY
    draw_row_padding(x, &y, row_width, padding_y, row_color,
                     side_flags, row_id, row_index++);
    draw_row(x, y, row_width, row_height, row_color, side_flags,
             row_id, row_index++);
    bool32 delete_entity_pressed = do_text_button(x + padding_x, y,
                                                  100.0f, row_height,
                                                  default_text_button_cancel_style, default_text_style,
                                                  "Delete Entity",
                                                  font_id,
                                                  false,
                                                  "entity_box_delete_entity");
    y += row_height;
    draw_row_padding(x, &y, row_width, padding_y, row_color,
                     side_flags | SIDE_BOTTOM, row_id, row_index++);
    if (delete_entity_pressed) {
        do_delete_entity(editor_state, editor_state->selected_entity_id);
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
    int32 font_id;
    Font font = get_font(asset_manager, editor_font_name, &font_id);
    
    x += padding_x;
    real32 new_level_button_width = 50.0f;
    bool32 new_level_clicked = do_text_button(x, y,
                                              new_level_button_width, button_height,
                                              default_text_button_style, default_text_style,
                                              "New",
                                              font_id, "new_level");
    if (new_level_clicked) {
        // TODO: do this
    }

    x += new_level_button_width + 1;

    bool32 just_loaded_level = editor_state->is_startup;

    real32 open_level_button_width = 60.0f;
    bool32 open_level_clicked = do_text_button(x, y, 
                                               open_level_button_width, button_height,
                                               default_text_button_style, default_text_style,
                                               "Open",
                                               font_id, "open_level");
    if (open_level_clicked) {
        Marker m = begin_region();
        char *absolute_filename = (char *) region_push(PLATFORM_MAX_PATH);
        
        if (platform_open_file_dialog(absolute_filename,
                                      LEVEL_FILE_FILTER_TITLE, LEVEL_FILE_FILTER_TYPE,
                                      PLATFORM_MAX_PATH)) {
            bool32 result = read_and_load_level(editor_state, absolute_filename);
            if (result) {
                just_loaded_level = true;
            } else {
                assert(!"Failed to load level.");
            }
#if 0
            Level_Info level_info;
            init_level_info(temp_region, &level_info);
            
            File_Data level_file = platform_open_and_read_file(temp_region, absolute_filename);
            bool32 result = Level_Loader::parse_level_info(temp_region, level_file, &level_info);

            if (result) {
                unload_level(editor_state);
                reset_editor(editor_state);
                load_level(editor_state, &level_info);
                just_loaded_level = true;
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
            "Level Name", font_id, default_text_style, "level_name_text_box_label");
    y += label_row_height;

    // level name text box
    draw_row(x, y, row_width, row_height, row_color, side_flags, row_id, row_index++);
    UI_Text_Box_Result level_name_result = do_text_box(x + padding_x, y,
                                                       row_width - padding_x*2, row_height,
                                                       editor_state->level.name, 64,
                                                       font_id,
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
            replace_with_copy((Allocator *) &editor_state->general_heap,
                              &editor_state->level.name, new_level_name);
        }
    }
    
    y += row_height;

#if 1
    bool32 level_name_is_valid = (!is_empty(editor_state->level.name) &&
                                  string_equals(editor_state->level.name, new_level_name));

    draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags, row_id, row_index);

    // save level button
    draw_row(x, y, row_width, button_height, row_color, side_flags, row_id, row_index++);

    real32 save_button_width = 50.0f;
    bool32 save_level_clicked = do_text_button(x + padding_x, y,
                                               save_button_width, button_height,
                                               default_text_button_style, default_text_style,
                                               "Save",
                                               font_id,
                                               !level_name_is_valid,
                                               "save_level");

    if (save_level_clicked) {
        assert(!is_empty(editor_state->level.name));

        Marker m = begin_region();
        char *filename = (char *) region_push(PLATFORM_MAX_PATH);

        if (editor_state->is_new_level) {
            bool32 has_filename = platform_open_save_file_dialog(filename,
                                                                 LEVEL_FILE_FILTER_TITLE, LEVEL_FILE_FILTER_TYPE,
                                                                 PLATFORM_MAX_PATH);

            if (has_filename) {
                export_level(asset_manager, &editor_state->level, filename);
                if (editor_state->level_filename.allocator) {
                    replace_with_copy((Allocator *) &editor_state->general_heap,
                                      &editor_state->level_filename, make_string(filename));
                }
                
                editor_state->is_new_level = false;
                
                add_message(Context::message_manager, make_string(SAVE_SUCCESS_MESSAGE));
            }
        } else {
            char *level_filename = to_char_array(temp_region, editor_state->level_filename);
            export_level(asset_manager, &editor_state->level, level_filename);
            add_message(Context::message_manager, make_string(SAVE_SUCCESS_MESSAGE));
        }
        
        end_region(m);
    }

    x += save_button_width + 1;
    real32 save_as_button_width = 110.0f;
    bool32 save_as_level_clicked = do_text_button(x + padding_x, y,
                                                  save_as_button_width, button_height,
                                                  default_text_button_style, default_text_style,
                                                  "Save As...",
                                                  font_id,
                                                  !level_name_is_valid,
                                                  "save_as_level");

    if (save_as_level_clicked) {
        assert(!is_empty(editor_state->level.name));

        Marker m = begin_region();
        char *filename = (char *) region_push(PLATFORM_MAX_PATH);

        bool32 has_filename = platform_open_save_file_dialog(filename,
                                                             LEVEL_FILE_FILTER_TITLE, LEVEL_FILE_FILTER_TYPE,
                                                             PLATFORM_MAX_PATH);

        if (has_filename) {
            export_level(asset_manager, &editor_state->level, filename);
            if (editor_state->level_filename.allocator) {
                replace_with_copy((Allocator *) &editor_state->general_heap,
                                  &editor_state->level_filename, make_string(filename));
            }
            editor_state->is_new_level = false;
            add_message(Context::message_manager, make_string(SAVE_SUCCESS_MESSAGE));
        }
        
        end_region(m);
    }
    x = initial_x;

    y += button_height;
    
#endif
    draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags | SIDE_BOTTOM, row_id, row_index);
}
