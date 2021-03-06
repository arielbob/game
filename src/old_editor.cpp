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
char *editor_font_name_bold = "calibri14b";
#endif

void deselect_entity(Editor_State *editor_state) {
    editor_state->selected_entity_type = ENTITY_NONE;
    editor_state->selected_entity_id = -1;
}

// for comparing current to new
bool32 selected_entity_changed(Editor_State *editor_state,
                               int32 new_entity_id, Entity_Type new_entity_type) {
    return ((new_entity_id != editor_state->selected_entity_id) ||
            (new_entity_type != editor_state->selected_entity_type));
}

// for comparing last to current
bool32 selected_entity_changed(Editor_State *editor_state) {
    return ((editor_state->selected_entity_id != editor_state->last_selected_entity_id) ||
            (editor_state->selected_entity_type != editor_state->last_selected_entity_type));
}

void reset_entity_editors(Editor_State *editor_state) {
    editor_state->open_window_flags = 0;
    editor_state->color_picker_parent = {};
}

Entity *allocate_and_copy_entity(Allocator *allocator, Entity *entity) {
    Entity *uncast_entity = NULL;
    if (entity->type == ENTITY_NORMAL) {
        Normal_Entity *entity_alloc = (Normal_Entity *) allocate(allocator, sizeof(Normal_Entity));
        *entity_alloc = *((Normal_Entity *) entity);
        uncast_entity = (Entity *) entity_alloc;
    } else if (entity->type == ENTITY_POINT_LIGHT) {
        Point_Light_Entity *entity_alloc = (Point_Light_Entity *) allocate(allocator, sizeof(Point_Light_Entity));
        *entity_alloc = *((Point_Light_Entity *) entity);
        uncast_entity = (Entity *) entity_alloc;
    } else {
        assert(!"Unhandled entity type");
    }

    return uncast_entity;
}

void start_entity_change(Editor_State *editor_state, Entity *entity) {
    assert(editor_state->old_entity == NULL); // make sure we haven't started an entity change already
    assert(entity);
    editor_state->old_entity = allocate_and_copy_entity(editor_state->history.allocator_pointer, entity);
}

void finalize_entity_change(Editor_State *editor_state, Level *level, Entity *entity) {
    assert(editor_state->old_entity);
    Entity *new_entity = allocate_and_copy_entity(editor_state->history.allocator_pointer, entity);
    Modify_Entity_Action action = make_modify_entity_action(editor_state->selected_entity_id,
                                                            editor_state->old_entity,
                                                            new_entity);
    editor_modify_entity(editor_state, level, action);
    editor_state->old_entity = NULL;
}

void start_or_finalize_entity_change(Game_State *game_state, UI_id element_id,
                                     Entity *entity) {
    Editor_State *editor_state = &game_state->editor_state;
    UI_Manager *ui_manager = &game_state->ui_manager;

    if (is_newly_active(ui_manager, element_id)) {
        editor_state->old_entity = allocate_and_copy_entity(editor_state->history.allocator_pointer, entity);
    } else if (is_newly_inactive(ui_manager, element_id)) {
        Entity *new_entity = allocate_and_copy_entity(editor_state->history.allocator_pointer, entity);
        Modify_Entity_Action action = make_modify_entity_action(editor_state->selected_entity_id,
                                                                editor_state->old_entity,
                                                                new_entity);
        editor_modify_entity(editor_state, &game_state->current_level, action);
        editor_state->old_entity = NULL;
    }
}

void start_material_change(Editor_State *editor_state, Material material) {
    editor_state->old_material = copy(editor_state->history.allocator_pointer, material);
}

void finalize_material_change(Editor_State *editor_state, Level *level, int32 material_id, Material material) {
    Material new_material = copy(editor_state->history.allocator_pointer, material);
    Modify_Material_Action action = make_modify_material_action(material_id,
                                                                editor_state->old_material, new_material);
    editor_modify_material(editor_state, level, action);
    editor_state->old_material = {};
}

void start_or_finalize_material_change(Editor_State *editor_state, UI_Manager *ui_manager,
                                       Level *level,
                                       UI_id element_id,
                                       int32 material_id, Material material) {
    if (is_newly_active(ui_manager, element_id)) {
        start_material_change(editor_state, material);
    } else if (is_newly_inactive(ui_manager, element_id)) {
        finalize_material_change(editor_state, level, material_id, material);
    }
}

#if 0
void start_texture_change(Editor_State *editor_state, Texture texture) {
    editor_state->old_texture = copy(editor_state->history.allocator_pointer, texture);
}

void finalize_texture_change(Editor_State *editor_state, Level *level, int32 texture_id, Texture texture) {
    Texture new_texture = copy(editor_state->history.allocator_pointer, texture);
    Modify_Texture_Action action = make_modify_texture_action(texture_id,
                                                              editor_state->old_texture, new_texture);
    editor_modify_texture(editor_state, level, action);
    editor_state->old_texture = {};
}

void start_or_finalize_texture_change(Editor_State *editor_state, UI_Manager *ui_manager,
                                      Level *level,
                                      UI_id element_id,
                                      int32 texture_id) {
    if (is_newly_active(ui_manager, element_id)) {
        start_texture_change(editor_state, texture);
    } else if (is_newly_inactive(ui_manager, element_id)) {
        finalize_texture_change(editor_state, level, texture_id, texture);
    }
}
#endif

void generate_texture_name(Level *level, String_Buffer *buffer) {
    int32 num_attempts = 0;
    while (num_attempts < MAX_TEXTURES + 1) {
        Marker m = begin_region();
        char *format = (num_attempts == 0) ? "New Texture" : "New Texture %d";
        char *buf = string_format((Allocator *) &memory.global_stack, buffer->size, format, num_attempts + 1);
        if (!texture_name_exists(level, make_string(buf))) {
            set_string_buffer_text(buffer, buf);
            end_region(m);
            return;
        }

        num_attempts++;
        end_region(m);
    }

    assert(!"Could not generate texture name.");
}

void handle_color_picker(Editor_State *editor_state, UI_Color_Picker_Result result) {
    if (result.should_hide) {
        editor_state->color_picker_parent = {};
    }
}

void generate_mesh_name(Asset_Manager *asset_manager, Level *level, String_Buffer *buffer) {
    int32 num_attempts = 0;
    while (num_attempts < MAX_MESHES + 1) {
        Marker m = begin_region();
        char *format = (num_attempts == 0) ? "New Mesh" : "New Mesh %d";
        char *buf = string_format((Allocator *) &memory.global_stack, buffer->size, format, num_attempts + 1);
        if (!mesh_name_exists(asset_manager, make_string(buf))) {
            set_string_buffer_text(buffer, buf);
            end_region(m);
            return;
        }

        num_attempts++;
        end_region(m);
    }

    assert(!"Could not generate mesh name.");
}

bool32 editor_add_mesh_press(Editor_State *editor_state, Asset_Manager *asset_manager,
                             Level *level) {
    Marker m = begin_region();
    char *absolute_filename = (char *) region_push(PLATFORM_MAX_PATH);
        
    Allocator *string_allocator = (Allocator *) level->string_pool_pointer;
    Allocator *filename_allocator = (Allocator *) level->filename_pool_pointer;

    if (platform_open_file_dialog(absolute_filename, PLATFORM_MAX_PATH)) {
        // FIXME: this leaks, i'm pretty sure
        String_Buffer new_mesh_name = make_string_buffer(string_allocator, 64);
        generate_mesh_name(asset_manager, level, &new_mesh_name);

        char *relative_filename = (char *) region_push(PLATFORM_MAX_PATH);
        platform_get_relative_path(absolute_filename, relative_filename, PLATFORM_MAX_PATH);
        String_Buffer new_mesh_filename = make_string_buffer(filename_allocator,
                                                             relative_filename, PLATFORM_MAX_PATH);

        Add_Mesh_Action action = make_add_mesh_action(make_string(relative_filename), make_string(new_mesh_name),
                                                      editor_state->selected_entity_type,
                                                      editor_state->selected_entity_id);
        editor_add_mesh(editor_state, asset_manager, level, action);

        //set_entity_mesh(asset_manager, entity, mesh_id);
#if 0
        Mesh new_mesh = add_level_mesh(asset_manager,
                                       make_string(relative_filename), make_string(new_mesh_name), &mesh_id);
        
        set_entity_mesh(asset_manager, entity, mesh_id);
        end_region(m);
#endif

        end_region(m);
        return true;
    }

    // TODO: error handling (after in-game console implementation)

    end_region(m);
    return false;
}

bool32 editor_add_texture_press(Editor_State *editor_state, Level *level,
                                int32 current_texture_id, int32 material_id) {
    Marker m = begin_region();
    char *absolute_filename = (char *) region_push(PLATFORM_MAX_PATH);
        
    if (platform_open_file_dialog(absolute_filename, PLATFORM_MAX_PATH)) {
        String_Buffer new_texture_name = make_string_buffer((Allocator *) &memory.global_stack,
                                                            TEXTURE_NAME_MAX_SIZE);
        generate_texture_name(level, &new_texture_name);

        char *relative_filename = (char *) region_push(PLATFORM_MAX_PATH);
        platform_get_relative_path(absolute_filename, relative_filename, PLATFORM_MAX_PATH);

        Add_Texture_Action action = make_add_texture_action(make_string(relative_filename),
                                                            make_string(new_texture_name),
                                                            material_id,
                                                            current_texture_id);
        editor_add_texture(editor_state, level, action);

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
    Asset_Manager *asset_manager = &game_state->asset_manager;

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
    Vec4 row_color = Editor_Constants::row_color;

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

    //Hash_Table<int32, Mesh> *mesh_table = &game_state->current_level.mesh_table;
    //Mesh_Type picked_mesh_type = Mesh_Type::NONE;
    int32 picked_mesh_id = -1;

    FOR_ENTRY_POINTERS(int, Mesh, game_state->asset_manager.mesh_table) {
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
                                        editor_font_name,
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

#if 0
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
#endif    

    if (picked_mesh_id >= 0) {
        start_entity_change(editor_state, selected_entity);
        set_entity_mesh(asset_manager, selected_entity, picked_mesh_id);
        finalize_entity_change(editor_state, &game_state->current_level, selected_entity);

        editor_state->open_window_flags = 0;
    }

    pop_layer(ui_manager);
}

void draw_texture_library(int32 material_id, Material *selected_material) {
    using namespace Context;
    Render_State *render_state = &game_state->render_state;

    push_layer(ui_manager);

    real32 padding_x = 20.0f;
    real32 padding_y = 20.0f;

    real32 x_gap = 10.0f;
    real32 y_gap = 10.0f;

    int32 num_items_per_row = 5;
    real32 item_width = 100.0f;
    real32 item_height = 100.0f;
    

    real32 window_width = padding_x * 2 + x_gap * (num_items_per_row - 1) + num_items_per_row*item_width;

    real32 title_row_height = 50.0f;
    Vec4 title_row_color = make_vec4(0.05f, 0.2f, 0.5f, 1.0f);
    Vec4 row_color = Editor_Constants::row_color;
    
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
    image_button_style.image_constraint_flags = (CONSTRAINT_FILL_BUTTON_WIDTH |
                                                 CONSTRAINT_FILL_BUTTON_HEIGHT |
        CONSTRAINT_KEEP_IMAGE_PROPORTIONS);
                                                 

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
            start_material_change(editor_state, *selected_material);
            selected_material->texture_id = picked_texture_id;
            finalize_material_change(editor_state, &game_state->current_level, material_id, *selected_material);
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
    Vec4 row_color = Editor_Constants::row_color;

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
        start_entity_change(editor_state, entity);
        set_entity_material(entity, picked_material_id);
        finalize_entity_change(editor_state, &game_state->current_level, entity);
        editor_state->open_window_flags = 0;
    }

    pop_layer(ui_manager);
};

#if 0
void open_color_picker(Editor_Color_Picker *editor_color_picker, UI_id parent_id, Vec4 initial_color) {
    editor_color_picker->parent_ui_id = parent_id;
    editor_color_picker->ui_state = make_color_picker_state(initial_color,
                                                            Editor_Constants::hsv_picker_width,
                                                            Editor_Constants::hsv_picker_height);
}
#endif

void draw_entity_box(Game_State *game_state, Controller_State *controller_state, int32 entity_id, Entity *entity) {
    int32 row_index = 0;

    UI_Manager *ui_manager = &game_state->ui_manager;
    Editor_State *editor_state = &game_state->editor_state;
    Asset_Manager *asset_manager = &game_state->asset_manager;

    real32 box_x = 5.0f;
    real32 box_y = 50.0f;

    real32 box_padding_x = 10.0f;
    real32 box_padding_y = 10.0f;

    Allocator *allocator = (Allocator *) &memory.frame_arena;

    Level *level = &game_state->current_level;
    //Mesh *mesh = get_mesh_pointer(game_state, level, entity->mesh_type, entity->mesh_id);
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
    real32 right_column_offset = padding_left + 130.0f;
    real32 small_spacing = 20.0f;

    real32 initial_row_height = 22.0f;
    real32 row_height = initial_row_height;
    real32 small_row_height = SMALL_ROW_HEIGHT;
    real32 row_width = 400.0f;

    Vec4 title_row_color = make_vec4(0.05f, 0.2f, 0.5f, 1.0f);
    Vec4 row_color = Editor_Constants::row_color;
    Vec4 darkened_row_color = Editor_Constants::darkened_row_color;

    real32 initial_x = box_x;
    real32 x = box_x;
    real32 y = box_y;

    real32 title_row_height = 30.0f;
    uint32 side_flags = SIDE_LEFT | SIDE_RIGHT;

    char *row_id = "entity_properties_title";

    draw_row(x, y, row_width, title_row_height, title_row_color,
             side_flags | SIDE_TOP, row_id, row_index++);
    draw_centered_text(x, y, row_width, title_row_height,
                       "Entity Properties", editor_font_name, text_style);
    y += title_row_height;
    draw_row_line(x, &y, row_width);
        
    char *buf;
    int32 buffer_size = 16;
    
    Allocator *persistent_string_allocator = (Allocator *) &memory.string64_pool;

    // POSITION
    draw_row(x, y, row_width, small_row_height, row_color, side_flags, row_id, row_index++);
    draw_v_centered_text(x+padding_left, y, small_row_height,
                         "Position", editor_font_name, text_style);
    // x
    //buf = string_format(allocator, buffer_size, "%f", transform.position.x);
    x += right_column_offset;
    real32 transform_button_width = 80.0f;

    UI_id x_slider, y_slider, z_slider;
    real32 new_x = do_slider(x, y,
                             transform_button_width, small_row_height,
                             editor_font_name,
                             transform.position.x,
                             default_slider_style, default_text_style,
                             "x", entity_id,
                             &x_slider);
    x += transform_button_width + 5.0f;

    real32 new_y = do_slider(x, y,
                             transform_button_width, small_row_height,
                             editor_font_name,
                             transform.position.y,
                             default_slider_style, default_text_style,
                             "y", entity_id,
                             &y_slider);
    x += transform_button_width + 5.0f;

    real32 new_z = do_slider(x, y,
                             transform_button_width, small_row_height,
                             editor_font_name,
                             transform.position.z,
                             default_slider_style, default_text_style,
                             "z", entity_id,
                             &z_slider);
    x += transform_button_width + 5.0f;

    update_entity_position(asset_manager, entity, make_vec3(new_x, new_y, new_z));
    start_or_finalize_entity_change(game_state, x_slider, entity);
    start_or_finalize_entity_change(game_state, y_slider, entity);
    start_or_finalize_entity_change(game_state, z_slider, entity);

    y += small_row_height;
    x = initial_x;

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
    x += right_column_offset;

    UI_id scale_x_slider, scale_y_slider, scale_z_slider;
    real32 new_scale_x = do_slider(x, y,
                                   transform_button_width, small_row_height,
                                   editor_font_name,
                                   transform.scale.x,
                                   default_slider_style, default_text_style,
                                   "scale_x", entity_id,
                                   &scale_x_slider);
    x += transform_button_width + 5.0f;

    real32 new_scale_y = do_slider(x, y,
                                   transform_button_width, small_row_height,
                                   editor_font_name,
                                   transform.scale.y,
                                   default_slider_style, default_text_style,
                                   "scale_y", entity_id,
                                   &scale_y_slider);
    x += transform_button_width + 5.0f;

    real32 new_scale_z = do_slider(x, y,
                                   transform_button_width, small_row_height,
                                   editor_font_name,
                                   transform.scale.z,
                                   default_slider_style, default_text_style,
                                   "scale_z", entity_id,
                                   &scale_z_slider);
    x += transform_button_width + 5.0f;

    update_entity_scale(asset_manager, entity, make_vec3(new_scale_x, new_scale_y, new_scale_z));
    start_or_finalize_entity_change(game_state, scale_x_slider, entity);
    start_or_finalize_entity_change(game_state, scale_y_slider, entity);
    start_or_finalize_entity_change(game_state, scale_z_slider, entity);

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
        draw_row_padding(x, &y, row_width, padding_top, row_color, side_flags,
                         row_id, row_index++);
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        char *mesh_label_string = "Mesh";
        if (mesh->type == Mesh_Type::PRIMITIVE) mesh_label_string = "Mesh (primitive)";
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
        x += choose_mesh_button_width;

        bool32 delete_mesh_pressed = do_text_button(x, y,
                                                    small_button_width, row_height,
                                                    default_text_button_cancel_style, default_text_style,
                                                    "-", editor_font_name,
                                                    mesh->type == Mesh_Type::PRIMITIVE,
                                                    "delete_mesh");

        if (delete_mesh_pressed) {
            assert(mesh->type == Mesh_Type::LEVEL);
            Delete_Mesh_Action action = make_delete_mesh_action(normal_entity->mesh_id,
                                                                make_string(mesh->filename),
                                                                make_string(mesh->name));
            editor_delete_mesh(editor_state, asset_manager, level, action);
#if 0
            level_delete_mesh(asset_manager, level, normal_entity->mesh_id);
#endif
            editor_state->editing_selected_entity_mesh = false;
        }

        x += small_button_width + padding_left;

        bool32 add_mesh_pressed = do_text_button(x, y,
                                                 small_button_width, row_height,
                                                 button_style, default_text_style,
                                                 "+", editor_font_name,
                                                 "add_mesh");
        x += small_button_width + padding_left;

        bool32 mesh_added = false;
        if (add_mesh_pressed) {
            mesh_added = editor_add_mesh_press(editor_state, asset_manager, &game_state->current_level);
            if (mesh_added) {
                editor_state->editing_selected_entity_mesh = true;
                mesh = get_mesh_pointer(asset_manager, normal_entity->mesh_id);
            }
        }

        real32 edit_mesh_button_width = row_width - (x - initial_x) - padding_right;
        bool32 edit_mesh_pressed = do_text_button(x, y,
                                                  edit_mesh_button_width, row_height,
                                                  button_style, default_text_style,
                                                  "Edit", editor_font_name,
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
            draw_v_centered_text(x + padding_left + Editor_Constants::x_nested_offset, y, row_height,
                                 "Mesh Name", editor_font_name, default_text_style);

            UI_Text_Box_Result result = do_text_box(x + right_column_offset, y,
                                                    edit_box_width, row_height,
                                                    &mesh->name, editor_font_name,
                                                    text_box_style, default_text_style,
                                                    true, mesh_added,
                                                    "mesh_name_text_box", normal_entity->mesh_id);
            if (result.submitted) {
                String new_name = make_string(result.buffer);
                if (is_empty(new_name)) {
                    add_message(&game_state->message_manager, make_string("Mesh name cannot be empty!"));
                } else if (string_contains(new_name,
                                           Editor_Constants::disallowed_chars,
                                           Editor_Constants::num_disallowed_chars)) {
                    add_message(&game_state->message_manager, make_string("Mesh name cannot contain {, }, or double quotes!"));
                } else if (!string_equals(make_string(mesh->name), new_name)) {
                    if (!mesh_name_exists(asset_manager, new_name)) {
                        Modify_Mesh_Action action = make_modify_mesh_action(mesh->type,
                                                                            normal_entity->mesh_id,
                                                                            result.buffer);
                        editor_modify_mesh(game_state, action);
                    } else {
                        add_message(&game_state->message_manager, make_string("Mesh name already exists!"));
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
        draw_v_centered_text(x + padding_left, y, row_height,
                             "Collider", editor_font_name, default_text_style);
        x += right_column_offset;

        char *collider_type_text = get_collider_type_string(normal_entity->collider.type);
        draw_v_centered_text(x, y, row_height,
                             collider_type_text, editor_font_name, default_text_style);
        y += row_height + 1;
        x = initial_x;

        // WALKABLE PROPERTY
        draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags,
                         row_id, row_index++);
        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(x + padding_left, y, row_height,
                             "Is Walkable", editor_font_name, default_text_style);
        x += right_column_offset;

        char *is_walkable_text = normal_entity->is_walkable ? "true" : "false";
        bool32 toggle_is_walkable_pressed = do_text_button(x, y,
                                                           100.0f, row_height,
                                                           button_style, default_text_style,
                                                           is_walkable_text,
                                                           editor_font_name,
                                                           false,
                                                           "toggle_is_walkable");
        y += row_height;
        x = initial_x;

        draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags | SIDE_BOTTOM,
                         row_id, row_index++);
        
        if (toggle_is_walkable_pressed) {
            start_entity_change(editor_state, (Entity *) normal_entity);
            normal_entity->is_walkable = !normal_entity->is_walkable;
            finalize_entity_change(editor_state, level, (Entity *) normal_entity);
        }

            
        // MATERIAL
        draw_row_padding(x, &y, row_width, padding_bottom, row_color,
                         side_flags, row_id, row_index++);

        draw_row(x, y, row_width, row_height, row_color, side_flags,
                 row_id, row_index++);
        draw_v_centered_text(x+padding_left, y, row_height,
                             "Material", editor_font_name, text_style);

        side_flags = SIDE_LEFT | SIDE_RIGHT;

        x += right_column_offset;
        Material *material = NULL;
        char *material_name = "";
        bool32 has_material = normal_entity->material_id >= 0;
        if (has_material) {
            material = get_material_pointer(level, normal_entity->material_id);
            material_name = to_char_array(allocator, material->name);
        }

        real32 choose_material_button_width = edit_box_width - small_button_width;
        if (!has_material) {
            choose_material_button_width += small_button_width;
        }

        bool32 choose_material_pressed = do_text_button(x, y,
                                                        choose_material_button_width, row_height,
                                                        button_style, default_text_style,
                                                        material_name,
                                                        editor_font_name, "choose_material");

        if (!editor_state->open_window_flags && choose_material_pressed) {
            editor_state->open_window_flags |= MATERIAL_LIBRARY_WINDOW;
        }
        x += choose_material_button_width;

        if (has_material) {
            bool32 delete_material_pressed = do_text_button(x, y,
                                                            small_button_width, row_height,
                                                            default_text_button_cancel_style, default_text_style,
                                                            "-", editor_font_name,
                                                            "delete_material");

            if (delete_material_pressed) {
                Delete_Material_Action action = make_delete_material_action(normal_entity->material_id, *material);
                editor_delete_material(editor_state, level, action);

                //level_delete_material(level, normal_entity->material_id);
                editor_state->editing_selected_entity_material = false;
            }

            x += small_button_width;
        }

        x += padding_left;

        real32 add_material_button_width = small_button_width;
        bool32 add_material_pressed = do_text_button(x, y,
                                                     add_material_button_width, row_height,
                                                     button_style, default_text_style,
                                                     "+", editor_font_name, "add_material");
        x += add_material_button_width + padding_left;

        if (add_material_pressed) {
            // update the material variable with the new material
            Add_Material_Action action = make_add_material_action(normal_entity->type, entity_id);
            int32 result_id = editor_add_material(editor_state, level, action);
            material = get_material_pointer(level, result_id);
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

        if (has_material && editor_state->editing_selected_entity_material) {
            draw_row_padding(x, &y, row_width, padding_y, row_color,
                             side_flags, row_id, row_index++);
            row_color = Editor_Constants::darkened_row_color;

            draw_row_padding(x, &y, row_width, padding_y, row_color,
                             side_flags, row_id, row_index++);

            UI_Text_Box_Style text_box_style = default_text_box_style;

            real32 x_nested_offset = Editor_Constants::x_nested_offset;
            real32 label_x = x + padding_left + x_nested_offset;


            // MATERIAL NAME
            draw_row(x, y, row_width, row_height, row_color, side_flags,
                     row_id, row_index++);
            draw_v_centered_text(label_x, y, row_height,
                                 "Material Name", editor_font_name, default_text_style);
            // NOTE: we pass in normal_entity->material_id as the text button's index, because if the material id
            //       changes, for example, from switching materials or creating a new material, AND the name
            //       editing panel is still open, then we want the state to be reset. since the material_id's are
            //       always different from each other (since they're created from an incrementing int), the text
            //       box state will automatically be regenerated if the entity's material changes.
            UI_Text_Box_Result material_name_text_box_result = do_text_box(x + right_column_offset, y,
                                                                           edit_box_width, row_height,
                                                                           &material->name, editor_font_name,
                                                                           text_box_style, default_text_style,
                                                                           true,
                                                                           "material_name_text_box",
                                                                           normal_entity->material_id);
            if (material_name_text_box_result.submitted) {
                String new_name = make_string(material_name_text_box_result.buffer);
                if (is_empty(new_name)) {
                    add_message(&game_state->message_manager, make_string("Material name cannot be empty!"));
                } else if (string_contains(new_name,
                                           Editor_Constants::disallowed_chars,
                                           Editor_Constants::num_disallowed_chars)) {
                    add_message(&game_state->message_manager, make_string("Material name cannot contain {, }, or double quotes!"));
                } else if (!string_equals(make_string(material->name), new_name)) {
                    if (!material_name_exists(level, new_name)) {
                        start_material_change(editor_state, *material);
                        copy_string(&material->name, new_name);
                        finalize_material_change(editor_state, level, normal_entity->material_id, *material);
                    } else {
                        add_message(&game_state->message_manager, make_string("Material name already exists!"));
                    }
                }
            }

#if 0
            String_Buffer text_result = do_text_box(x + right_column_offset, y,
                                                    edit_box_width, row_height,
                                                    &material->name, editor_font_name,
                                                    text_box_style, default_text_style,
                                                    "material_name_text_box");
#endif

            y += row_height;
            draw_row_padding(x, &y, row_width, padding_y, row_color,
                             side_flags, row_id, row_index++);

            // TEXTURE
            char *texture_name = "";
            if (material->texture_id >= 0) {
                Texture texture = get_texture(level, material->texture_id);
                texture_name = to_char_array(allocator, texture.name);
            }
            draw_row(x, y, row_width, row_height, row_color, side_flags,
                     row_id, row_index++);
            draw_v_centered_text(label_x, y, row_height,
                                 "Texture", editor_font_name, text_style);

            x += right_column_offset;
            bool32 has_texture = material->texture_id >= 0;

            real32 choose_texture_button_width = edit_box_width;
            if (has_texture) choose_texture_button_width -= small_button_width;

            bool32 choose_texture_pressed = do_text_button(x, y,
                                                           choose_texture_button_width, row_height,
                                                           button_style, default_text_style,
                                                           texture_name, editor_font_name, "choose_texture");
            if (!editor_state->open_window_flags && choose_texture_pressed) {
                editor_state->open_window_flags |= TEXTURE_LIBRARY_WINDOW;
            }
            x += choose_texture_button_width;

            if (has_texture) {
                Texture *texture = get_texture_pointer(level, material->texture_id);
                bool32 delete_texture_pressed = do_text_button(x, y,
                                                               small_button_width, row_height,
                                                               default_text_button_cancel_style, default_text_style,
                                                               "-", editor_font_name, "delete_texture");
                if (delete_texture_pressed) {
                    Delete_Texture_Action action = make_delete_texture_action(material->texture_id,
                                                                              make_string(texture->filename),
                                                                              make_string(texture->name));
                    editor_delete_texture(editor_state, level, action);
#if 0
                    level_delete_texture(level, material->texture_id);
                    
#endif
                    has_texture = false;
                    editor_state->editing_selected_entity_texture = false;
                }

                x += small_button_width;
            }
            

            x += padding_left;

            bool32 add_texture_pressed = do_text_button(x, y,
                                                        small_button_width, row_height,
                                                        button_style, default_text_style,
                                                        "+", editor_font_name, "add_texture");
            if (add_texture_pressed) {
                bool32 texture_added = editor_add_texture_press(editor_state, &game_state->current_level,
                                                                normal_entity->material_id, material->texture_id);
                if (texture_added) {
                    editor_state->editing_selected_entity_texture = true;
                }
            }

            x += small_button_width + padding_left;
            real32 edit_texture_button_width = row_width - (x - initial_x) - padding_right;
            bool32 edit_texture_pressed = do_text_button(x, y,
                                                          edit_texture_button_width, row_height,
                                                          button_style, default_text_style,
                                                          "Edit", editor_font_name,
                                                          !has_texture,
                                                          "edit_texture");
    
            x = initial_x;
            y += row_height;

            if (edit_texture_pressed) {
                editor_state->editing_selected_entity_texture = !editor_state->editing_selected_entity_texture;
            }

            if (has_texture && editor_state->editing_selected_entity_texture) {
                Texture *texture = get_texture_pointer(&game_state->current_level, material->texture_id);
                draw_row_padding(x, &y, row_width, padding_y, row_color,
                                 side_flags, row_id, row_index++);

                // TEXTURE NAME
                draw_row(x, y, row_width, row_height, darkened_row_color, side_flags,
                         row_id, row_index++);
                draw_v_centered_text(label_x + x_nested_offset, y, row_height,
                                     "Texture Name", editor_font_name, default_text_style);
                UI_Text_Box_Result result = do_text_box(x + right_column_offset, y,
                                                        edit_box_width, row_height,
                                                        &texture->name, editor_font_name,
                                                        text_box_style, default_text_style,
                                                        true,
                                                        "texture_name_text_box", material->texture_id);

                if (result.submitted) {
                    String new_name = make_string(result.buffer);
                    if (is_empty(new_name)) {
                        add_message(&game_state->message_manager, make_string("Texture name cannot be empty!"));
                    } else if (string_contains(new_name,
                                               Editor_Constants::disallowed_chars,
                                               Editor_Constants::num_disallowed_chars)) {
                        add_message(&game_state->message_manager, make_string("Texture name cannot contain {, }, or double quotes!"));
                    } else if (!string_equals(make_string(texture->name), new_name)) {
                        if (!texture_name_exists(level, new_name)) {
                            Modify_Texture_Action action = make_modify_texture_action(material->texture_id,
                                                                                      result.buffer);
                            editor_modify_texture(editor_state, level, action);
                            //copy_string(&texture->name, new_name);
                        } else {
                            add_message(&game_state->message_manager, make_string("Texture name already exists!"));
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
                                 "Gloss", editor_font_name, text_style);
            UI_id gloss_slider;
            material->gloss = do_slider(x+right_column_offset, y,
                                        edit_box_width, row_height,
                                        editor_font_name,
                                        0.0f, 100.0f, material->gloss,
                                        default_slider_style, default_text_style,
                                        "edit_material_gloss_slider", normal_entity->material_id,
                                        &gloss_slider);
            y += row_height;
            draw_row_padding(x, &y, row_width, padding_y, row_color,
                             side_flags, row_id, row_index++);

            // COLOR OVERRIDE
            draw_row(x, y, row_width, row_height, row_color, side_flags,
                     row_id, row_index++);
            buf = string_format(allocator, buffer_size, "%f", material->gloss);
            draw_v_centered_text(label_x, y, row_height,
                                 "Color Override", editor_font_name, text_style);
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

                if (result.started) {
                    start_material_change(editor_state, *material);
                } else if (result.submitted) {
                    material->color_override = result.color;
                    finalize_material_change(editor_state, level, normal_entity->material_id, *material);
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
        draw_v_centered_text(x + padding_left, y, row_height,
                             "Light Color", editor_font_name, text_style);
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
            point_light->light_color = truncate_v4_to_v3(result.color);
            if (result.should_hide) {
                editor_state->color_picker_parent = {};
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
        draw_v_centered_text(x + padding_left, y, row_height,
                             "Falloff Start", editor_font_name, text_style);
        UI_id falloff_start_slider;
        point_light->falloff_start = do_slider(x+right_column_offset, y,
                                               edit_box_width, row_height,
                                               editor_font_name,
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
        draw_v_centered_text(x + padding_left, y, row_height,
                             "Falloff Distance", editor_font_name, text_style);
        UI_id falloff_end_slider;
        point_light->falloff_end = do_slider(x+right_column_offset, y,
                                             edit_box_width, row_height,
                                             editor_font_name,
                                             0.0f, 100.0f, point_light->falloff_end,
                                             default_slider_style, default_text_style,
                                             "edit_point_light_falloff_end_slider", entity_id,
                                             &falloff_end_slider);
        y += row_height;
        draw_row_padding(x, &y, row_width, padding_y, row_color,
                         side_flags | SIDE_BOTTOM, row_id, row_index++);
    }

    // DELETE ENTITY
    draw_row_padding(x, &y, row_width, padding_y, row_color,
                     side_flags, row_id, row_index++);
    draw_row(x, y, row_width, row_height, row_color, side_flags,
             row_id, row_index++);
    bool32 delete_entity_pressed = do_text_button(x + padding_left, y,
                                                  100.0f, row_height,
                                                  default_text_button_cancel_style, default_text_style,
                                                  "Delete Entity",
                                                  editor_font_name,
                                                  false,
                                                  "entity_box_delete_entity");
    y += row_height;
    draw_row_padding(x, &y, row_width, padding_y, row_color,
                     side_flags | SIDE_BOTTOM, row_id, row_index++);
    if (delete_entity_pressed) {
        editor_delete_entity(editor_state, level, editor_state->selected_entity_type, editor_state->selected_entity_id);
#if 0
        level_delete_entity(level, editor_state->selected_entity_type, editor_state->selected_entity_id);
        deselect_entity(editor_state);
#endif
    }
}

void draw_level_box(Game_State *game_state, Controller_State *controller_state,
                    real32 x, real32 y) {
    UI_Manager *ui_manager = &game_state->ui_manager;
    Editor_State *editor_state = &game_state->editor_state;
    Asset_Manager *asset_manager = &game_state->asset_manager;
    
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

    Font font = get_font(game_state, editor_font_name);
    
    x += padding_x;
    real32 new_level_button_width = 50.0f;
    bool32 new_level_clicked = do_text_button(x, y,
                                              new_level_button_width, button_height,
                                              default_text_button_style, default_text_style,
                                              "New",
                                              editor_font_name, "new_level");
    if (new_level_clicked) {
        new_level(asset_manager, &game_state->current_level);
        editor_state->selected_entity_id = -1;
        editor_state->is_new_level = true;
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
    UI_Text_Box_Result level_name_result = do_text_box(x + padding_x,
                                                       y,
                                                       row_width - padding_x*2, row_height,
                                                       &game_state->current_level.name, editor_font_name,
                                                       default_text_box_style, default_text_style,
                                                       true, just_loaded_level,
                                                       "level_name_text_box");
    String new_level_name = make_string(level_name_result.buffer);
    if (level_name_result.submitted) {
        if (is_empty(new_level_name)) {
            add_message(&game_state->message_manager, make_string("Level name cannot be empty!"));
        } else if (string_contains(new_level_name,
                                   Editor_Constants::disallowed_chars,
                                   Editor_Constants::num_disallowed_chars)) {
            add_message(&game_state->message_manager, make_string("Level name cannot contain {, }, or double quotes!"));
        } else {
            copy_string(&game_state->current_level.name, new_level_name);
        }
    }

    bool32 level_name_is_valid = (!is_empty(game_state->current_level.name) &&
                                  string_equals(make_string(game_state->current_level.name), new_level_name));

    y += row_height;
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
    
    draw_row_padding(x, &y, row_width, padding_y, row_color, side_flags | SIDE_BOTTOM, row_id, row_index);
}

void draw_editor_ui(Game_State *game_state, Controller_State *controller_state) {
    Editor_State *editor_state = &game_state->editor_state;
    UI_Manager *ui_manager = &game_state->ui_manager;
    Render_State *render_state = &game_state->render_state;

    if (editor_state->selected_entity_id >= 0) {
        Entity *selected_entity = get_selected_entity(game_state);
        draw_entity_box(game_state, controller_state, editor_state->selected_entity_id, selected_entity);

        if (editor_state->open_window_flags & MATERIAL_LIBRARY_WINDOW) {
            draw_material_library(game_state, controller_state, selected_entity);
        } else if (editor_state->open_window_flags & TEXTURE_LIBRARY_WINDOW) {
            int32 material_id;
            Material *selected_material = get_entity_material(&game_state->current_level,
                                                              selected_entity, &material_id);
            draw_texture_library(material_id, selected_material);
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

    bool32 add_normal_entity_clicked = do_text_button(render_state->display_output.width - sidebar_button_width, y,
                                                      sidebar_button_width, button_height,
                                                      default_text_button_style, default_text_style,
                                                      "Add Normal Entity",
                                                      button_font_name, "add_entity");
    y += button_height + button_gap;
    if (add_normal_entity_clicked) {
        Add_Normal_Entity_Action action = make_add_normal_entity_action();
        editor_add_normal_entity(editor_state, game_state, action);
    }

    bool32 add_point_light_entity_clicked = do_text_button(render_state->display_output.width - sidebar_button_width, y,
                                                      sidebar_button_width, button_height,
                                                      default_text_button_style, default_text_style,
                                                      "Add Point Light Entity",
                                                      button_font_name, "add_point_light_entity");
    y += button_height + button_gap;
    if (add_point_light_entity_clicked) {
        Add_Point_Light_Entity_Action action = make_add_point_light_entity_action();
        editor_add_point_light_entity(editor_state, game_state, action);
    }

    bool32 toggle_colliders_clicked = do_text_button(render_state->display_output.width - sidebar_button_width, y,
                                                     sidebar_button_width, button_height,
                                                     default_text_button_style, default_text_style,
                                                     editor_state->show_colliders ?
                                                     "Hide Colliders" : "Show Colliders",
                                                     button_font_name, "toggle_show_colliders");

    if (toggle_colliders_clicked) {
        editor_state->show_colliders = !editor_state->show_colliders;
    }

    y += button_height + button_height;

    int32 num_history_entries = history_get_num_entries(&editor_state->history);
    bool32 undo_clicked = do_text_button(render_state->display_output.width - sidebar_button_width, y,
                                         0.5f * sidebar_button_width - 1, button_height,
                                         default_text_button_style, default_text_style,
                                         "Undo",
                                         button_font_name,
                                         editor_state->history.num_undone == num_history_entries,
                                         "editor_undo");
    if (undo_clicked) {
        history_undo(game_state, &editor_state->history);
    }

    bool32 redo_clicked = do_text_button(render_state->display_output.width - 0.5f * sidebar_button_width, y,
                                         0.5f * sidebar_button_width, button_height,
                                         default_text_button_style, default_text_style,
                                         "Redo",
                                         button_font_name,
                                         num_history_entries == 0 || editor_state->history.num_undone == 0,
                                         "editor_redo");
    if (redo_clicked) {
        history_redo(game_state, &editor_state->history);
    }

    if (!editor_state->is_new_level) {
        char *filename_buf = to_char_array((Allocator *) &memory.frame_arena, editor_state->current_level_filename);
        char *buf = string_format((Allocator *) &memory.frame_arena, PLATFORM_MAX_PATH + 32,
                                  "current level: %s", filename_buf);
        do_text(ui_manager,
                5.0f, render_state->display_output.height - 9.0f,
                buf, editor_font_name, default_text_style, "editor_current_level_filename");
    }

#if 0
    bool32 add_message_clicked = do_text_button(render_state->display_output.width - sidebar_button_width,
                                                render_state->display_output.height - button_height,
                                                sidebar_button_width, button_height,
                                                default_text_button_style, default_text_style,
                                                "Add Message",
                                                button_font_name, "add_message");

    static int32 num_messages_added_so_far = 0;
    if (add_message_clicked) {
        num_messages_added_so_far++;

        Marker m = begin_region();
        // store it in temp
        char *buf = string_format((Allocator *) &memory.global_stack, 64, "Message %d", num_messages_added_so_far);
        // copy it
        String string = make_string((Allocator *) &memory.string64_pool, buf);
        end_region(m);

        add_message(&game_state->message_manager, string);
    }
#endif


#if 0
    Vec3 closest_point_below = make_vec3();
    bool32 found_closest_point = false;
    Vec3 camera_position = render_state->camera.position;
    Level *level = &game_state->current_level;

    {
        FOR_VALUE_POINTERS(int32, Normal_Entity, level->normal_entity_table) {
            Normal_Entity *entity = value;
            if (entity->is_walkable) {
                Mesh mesh = get_mesh(game_state, level, entity->mesh_type, entity->mesh_id);
                Vec3 result;
                if (closest_point_below_on_mesh(camera_position, mesh, entity->transform, &result)) {
                    if (!found_closest_point) {
                        closest_point_below = result;
                        found_closest_point = true;
                    } else {
                        if (result.y > closest_point_below.y) {
                            closest_point_below = result;
                        }
                    }
                }
            }
        }
    }

    char *buf;
    if (found_closest_point) {
        buf = string_format((Allocator *) &memory.frame_arena, 128,
                            "closest point on triangle below (%f, %f, %f): (%f, %f, %f)",
                            camera_position.x, camera_position.y, camera_position.z,
                            closest_point_below.x, closest_point_below.y, closest_point_below.z);
    } else {
        buf = string_format((Allocator *) &memory.frame_arena, 128,
                            "closest point on triangle below (%f, %f, %f): None",
                            camera_position.x, camera_position.y, camera_position.z,
                            closest_point_below.x, closest_point_below.y, closest_point_below.z);
    }
    
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 16.0f,
            buf, editor_font_name, default_text_style, "highest_triangle_point_below");
#endif
    char *buf = string_format((Allocator *) &memory.frame_arena, 64, "heap size: %d", ui_manager->heap_pointer->size);
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 62.0f,
            buf, editor_font_name, default_text_style, "heap size");

    buf = string_format((Allocator *) &memory.frame_arena, 64, "heap used: %d", ui_manager->heap_pointer->used);
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 48.0f,
            buf, editor_font_name, default_text_style, "heap used");

    buf = string_format((Allocator *) &memory.frame_arena, 64, "editor history heap used: %d",
                        memory.editor_history_heap.used);
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 90.0f,
            buf, editor_font_name, default_text_style, "editor history heap size");

#if 0
    String_Buffer history_buf = make_string_buffer((Allocator *) &memory.frame_arena, 128);
    append_string(&history_buf, "[ ");
    for (int32 i = 0; i < MAX_EDITOR_HISTORY_ENTRIES; i++) {
        if (i == editor_state->history.start_index) {
            append_string(&history_buf, "start ");
        }

        if (i == editor_state->history.end_index) {
            append_string(&history_buf, "end ");
        }
        
        if (i == editor_state->history.current_index) {
            append_string(&history_buf, "current ");
        }

        if (editor_state->history.entries[i] == NULL) {
            append_string(&history_buf, "_ ");
        } else {
            append_string(&history_buf, "e ");

            if (editor_state->history.entries[i]->type == ACTION_ADD_NORMAL_ENTITY) {
                Add_Normal_Entity_Action *action = (Add_Normal_Entity_Action *) editor_state->history.entries[i];
                Marker m = begin_region();
                char *whatever = string_format((Allocator *) &memory.global_stack, 8, "(%d) ", action->entity_id);
                append_string(&history_buf, whatever);
                end_region(m);
            }

        }
        
        if (i < MAX_EDITOR_HISTORY_ENTRIES - 1) {
            append_string(&history_buf, " | ");
        }
        
    }
    append_string(&history_buf, "]");

    buf = to_char_array((Allocator *) &memory.frame_arena, history_buf);
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 114.0f,
            buf, editor_font_name, default_text_style, "editor history");
#endif

    buf = string_format((Allocator *) &memory.frame_arena, 64, "num_undone: %d", editor_state->history.num_undone);
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 129.0f,
            buf, editor_font_name, default_text_style, "num undone");

    buf = string_format((Allocator *) &memory.frame_arena, 64, "num_history_entries: %d", num_history_entries);
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 143.0f,
            buf, editor_font_name, default_text_style, "num history entries");

    Heap_Allocator *history_heap = &memory.editor_history_heap;
    String_Buffer history_buf = make_string_buffer((Allocator *) &memory.frame_arena, 512);
    append_string(&history_buf, "[ ");

    Heap_Block *block = history_heap->first_block;
    while (block) {
        Marker m = begin_region();
        char *whatever = string_format((Allocator *) &memory.global_stack, 16, "| %d | ", block->size);
        append_string(&history_buf, whatever);
        end_region(m);

        block = block->next;
    }

    append_string(&history_buf, "]");

    buf = to_char_array((Allocator *) &memory.frame_arena, history_buf);
    do_text(ui_manager,
            5.0f, render_state->display_output.height - 114.0f,
            buf, editor_font_name, default_text_style, "editor history");
}

int32 pick_entity(Game_State *game_state, Ray cursor_ray, Entity *entity_result, int32 *index_result) {
    Editor_State *editor_state = &game_state->editor_state;
    Asset_Manager *asset_manager = &game_state->asset_manager;

    Level *level = &game_state->current_level;

    Entity *picked_entity = NULL;
    int32 entity_id = -1;

    real32 t_min = FLT_MAX;
    {
        real32 aabb_t;
        FOR_ENTRY_POINTERS(int32, Normal_Entity, level->normal_entity_table) {
            Normal_Entity *entity = &entry->value;
            Mesh mesh = get_mesh(asset_manager, entity->mesh_id);
            if (ray_intersects_aabb(cursor_ray, entity->transformed_aabb, &aabb_t) && (aabb_t < t_min)) {
                Ray_Intersects_Mesh_Result result;
                if (ray_intersects_mesh(cursor_ray, mesh, entity->transform, true, &result) &&
                    (result.t < t_min)) {
                    t_min = result.t;
                    entity_id = entry->key;
                    picked_entity = (Entity *) entity;
                }
            }
        }
    }

    {
        Basis camera_basis = game_state->render_state.camera.current_basis;
        Vec3 plane_normal = -camera_basis.forward;
        FOR_ENTRY_POINTERS(int32, Point_Light_Entity, level->point_light_entity_table) {
            Point_Light_Entity *entity = &entry->value;
            
            real32 plane_d = dot(entity->transform.position, plane_normal);
            real32 t;
            if (ray_intersects_plane(cursor_ray, plane_normal, plane_d, &t)) {
                Vec3 hit_position = cursor_ray.origin + t*cursor_ray.direction;
                real32 plane_space_hit_x = dot(hit_position - entity->transform.position, camera_basis.right);
                real32 plane_space_hit_y = dot(hit_position - entity->transform.position, camera_basis.up);

                real32 icon_side_length = Editor_Constants::point_light_side_length;
                real32 offset = 0.5f * icon_side_length;
                if (plane_space_hit_x > -offset && plane_space_hit_x < offset &&
                    plane_space_hit_y > -offset && plane_space_hit_y < offset) {
                    if (t < t_min) {
                        t_min = t;
                        entity_id = entry->key;
                        picked_entity = (Entity *) entity;
                    }
                }
            }
        }
    }

    if (entity_id >= 0) {
        *entity_result = *picked_entity;
        *index_result = entity_id;
        return true;
    }

    return false;
}

bool32 is_translation(Gizmo_Handle gizmo_axis) {
    return (gizmo_axis == GIZMO_TRANSLATE_X ||
            gizmo_axis == GIZMO_TRANSLATE_Y ||
            gizmo_axis == GIZMO_TRANSLATE_Z);
}

bool32 is_scale(Gizmo_Handle gizmo_axis) {
    return (gizmo_axis == GIZMO_SCALE_X ||
            gizmo_axis == GIZMO_SCALE_Y ||
            gizmo_axis == GIZMO_SCALE_Z);
}

bool32 is_rotation(Gizmo_Handle gizmo_axis) {
    return (gizmo_axis == GIZMO_ROTATE_X ||
            gizmo_axis == GIZMO_ROTATE_Y ||
            gizmo_axis == GIZMO_ROTATE_Z);
}

Vec3 get_gizmo_transform_axis(Transform_Mode transform_mode, Gizmo_Handle gizmo_handle, Transform transform) {
    assert(gizmo_handle != GIZMO_HANDLE_NONE);

    uint32 axis;

    if (gizmo_handle == GIZMO_TRANSLATE_X ||
        gizmo_handle == GIZMO_SCALE_X ||
        gizmo_handle == GIZMO_ROTATE_X) {
        // x
        axis = 0;
    } else if (gizmo_handle == GIZMO_TRANSLATE_Y ||
               gizmo_handle == GIZMO_SCALE_Y ||
               gizmo_handle == GIZMO_ROTATE_Y) {
        // y
        axis = 1;
    } else {
        // z
        axis = 2;
    }
     
    if (transform_mode == TRANSFORM_GLOBAL) {
        if (axis == 0) {
            return x_axis;
        } else if (axis == 1) {
            return y_axis;
        } else {
            return z_axis;
        }
    } else {
        // local transform

        // TODO: maybe cache this model matrix?
        Mat4 entity_model_matrix = get_model_matrix(transform);

        if (axis == 0) {
            return normalize(truncate_v4_to_v3(entity_model_matrix.col1));
        } else if (axis == 1) {
            return  normalize(truncate_v4_to_v3(entity_model_matrix.col2));
        } else {
            return normalize(truncate_v4_to_v3(entity_model_matrix.col3));
        }
    }
}

Gizmo_Handle pick_gizmo(Game_State *game_state, Ray cursor_ray, real32 *t_result) {
    Editor_State *editor_state = &game_state->editor_state;
    Asset_Manager *asset_manager = &game_state->asset_manager;

    Entity *entity = get_selected_entity(game_state);
    Gizmo gizmo = editor_state->gizmo;

    Transform_Mode transform_mode = editor_state->transform_mode;

    Transform x_transform, y_transform, z_transform;
    // TODO: maybe add some scale here? for a bit of extra space to click on the gizmo.
    if (transform_mode == TRANSFORM_GLOBAL) {
        x_transform = gizmo.transform;
        x_transform.rotation = make_quaternion();
        y_transform = gizmo.transform;
        y_transform.rotation = make_quaternion(90.0f, z_axis);
        z_transform = gizmo.transform;
        z_transform.rotation = make_quaternion(-90.0f, y_axis);
    } else {
        // local transform
        x_transform = gizmo.transform;
        y_transform = gizmo.transform;
        y_transform.rotation = gizmo.transform.rotation*make_quaternion(90.0f, z_axis);
        z_transform = gizmo.transform;
        z_transform.rotation = gizmo.transform.rotation*make_quaternion(-90.0f, y_axis);
    }

    Gizmo_Handle picked_handle = GIZMO_HANDLE_NONE;

    Transform gizmo_handle_transforms[3] = { x_transform, y_transform, z_transform };

    // check ray against translation arrows
    Gizmo_Handle gizmo_translation_handles[3] = { GIZMO_TRANSLATE_X, GIZMO_TRANSLATE_Y, GIZMO_TRANSLATE_Z };
    assert(gizmo.arrow_mesh_id >= 0);
    Mesh arrow_mesh = get_mesh(asset_manager, gizmo.arrow_mesh_id);

    real32 t_min = FLT_MAX;
    for (int32 i = 0; i < 3; i++) {
        Transform gizmo_handle_transform = gizmo_handle_transforms[i];
        Ray_Intersects_Mesh_Result result;
        if (ray_intersects_mesh(cursor_ray, arrow_mesh, gizmo_handle_transform, true, &result) &&
            (result.t < t_min)) {
            t_min = result.t;
            picked_handle = gizmo_translation_handles[i];
        }
    }

    // check ray against scale cube handles
    Gizmo_Handle gizmo_scale_handles[3] = { GIZMO_SCALE_X, GIZMO_SCALE_Y, GIZMO_SCALE_Z };
    assert(gizmo.cube_mesh_id >= 0);
    Mesh gizmo_cube_mesh = get_mesh(asset_manager, gizmo.cube_mesh_id);

    for (int32 i = 0; i < 3; i++) {
        Transform gizmo_handle_transform = gizmo_handle_transforms[i];
        Ray_Intersects_Mesh_Result result;
        if (ray_intersects_mesh(cursor_ray, gizmo_cube_mesh, gizmo_handle_transform, true, &result) &&
            (result.t < t_min)) {
            t_min = result.t;
            picked_handle = gizmo_scale_handles[i];
        }
    }

    // check ray against rotation rings
    Gizmo_Handle gizmo_rotation_handles[3] = { GIZMO_ROTATE_X, GIZMO_ROTATE_Y, GIZMO_ROTATE_Z };
    assert(gizmo.ring_mesh_id >= 0);
    Mesh ring_mesh = get_mesh(asset_manager, gizmo.ring_mesh_id);

    for (int32 i = 0; i < 3; i++) {
        Transform gizmo_handle_transform = gizmo_handle_transforms[i];
        Ray_Intersects_Mesh_Result result;
        if (ray_intersects_mesh(cursor_ray, ring_mesh, gizmo_handle_transform, true, &result) &&
            (result.t < t_min)) {
            t_min = result.t;
            picked_handle = gizmo_rotation_handles[i];
        }
    }

    *t_result = t_min;

    return picked_handle;
}

Vec3 do_gizmo_translation(Camera *camera, Editor_State *editor_state, Ray cursor_ray) {
    real32 t;
    Vec3 camera_forward = camera->current_basis.forward;

    Vec3 initial_hit;
    if (editor_state->transform_mode == TRANSFORM_GLOBAL) {
        initial_hit = editor_state->global_initial_gizmo_hit;
    } else {
        initial_hit = editor_state->local_initial_gizmo_hit;
    }

    Ray transform_ray = make_ray(initial_hit,
                                 editor_state->gizmo_transform_axis);
    Plane plane = get_plane_containing_ray(transform_ray, camera_forward);
    bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);

    Vec3 new_position = editor_state->old_entity->transform.position;

    // this will always intersect unless your FOV is >= 180 degrees
    if (intersects_plane) {
        Vec3 intersect_point = cursor_ray.origin + t*cursor_ray.direction;
        real32 delta_length = dot(intersect_point - initial_hit,
                                  editor_state->gizmo_transform_axis);
        Vec3 delta = editor_state->gizmo_transform_axis * delta_length;
        new_position += delta;
    }

    return new_position;
}

// TODO: local scaling should just use x, y, z standard basis vectors, and not the entity's transformed basis
//       vectors.
// TODO: global scaling should first get the model matrix, then scale it using standard basis, then extract the
//       scale from the scaled model matrix.
Vec3 do_gizmo_scale(Camera *camera, Editor_State *editor_state, Ray cursor_ray) {
    real32 t;
    Vec3 camera_forward = camera->current_basis.forward;

    Vec3 initial_hit;
    if (editor_state->transform_mode == TRANSFORM_GLOBAL) {
        initial_hit = editor_state->global_initial_gizmo_hit;
    } else {
        initial_hit = editor_state->local_initial_gizmo_hit;
    }

    Gizmo_Handle gizmo_handle = editor_state->selected_gizmo_handle;
    Vec3 scale_transform_axis = {};
    if (gizmo_handle == GIZMO_SCALE_X) {
        scale_transform_axis = x_axis;
    } else if (gizmo_handle == GIZMO_SCALE_Y) {
        scale_transform_axis = y_axis;
    } else {
        scale_transform_axis = z_axis;
    }

    // we still use the transform ray here since the selected gizmo axis is lined up with the transform ray.
    Ray transform_ray = make_ray(initial_hit,
                                 editor_state->gizmo_transform_axis);
    Plane plane = get_plane_containing_ray(transform_ray, camera_forward);
    bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);

    Transform original_transform = editor_state->old_entity->transform;
    Vec3 new_scale = original_transform.scale;

    // this will always intersect unless your FOV is >= 180 degrees
    if (intersects_plane) {
        Vec3 intersect_point = cursor_ray.origin + t*cursor_ray.direction;
        real32 delta_length = dot(intersect_point - initial_hit,
                                  editor_state->gizmo_transform_axis);
        Vec3 delta = scale_transform_axis * delta_length;
        new_scale += delta;
        if (editor_state->transform_mode == TRANSFORM_LOCAL) {
            return new_scale;
        } else {
            Mat4 model = get_model_matrix(original_transform);
            delta = make_vec3(1.0f, 1.0f, 1.0f) + delta;

            // NOTE: when delta goes below the zero vector, the entity grows instead of shrinks. i'm pretty sure
            //       it's because when we call distance() on the columns of the scaled model matrix, it does not
            //       give signed values. i think this is fine.
            Mat4 scale_matrix = make_scale_matrix(delta);
            Mat4 scaled_model = scale_matrix * model;
            new_scale = make_vec3(distance(truncate_v4_to_v3(scaled_model.col1)),
                                  distance(truncate_v4_to_v3(scaled_model.col2)),
                                  distance(truncate_v4_to_v3(scaled_model.col3)));
        }
    }

    return new_scale;
}

Quaternion do_gizmo_rotation(Camera *camera, Editor_State *editor_state, Ray cursor_ray) {
    real32 t;
    Vec3 camera_forward = camera->current_basis.forward;
    Vec3 center = editor_state->gizmo.transform.position;
    Plane plane = { dot(center, editor_state->gizmo_transform_axis),
                    editor_state->gizmo_transform_axis };
    bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);

    Vec3 initial_hit;
    if (editor_state->transform_mode == TRANSFORM_GLOBAL) {
        initial_hit = editor_state->global_initial_gizmo_hit;
    } else {
        initial_hit = editor_state->local_initial_gizmo_hit;
    }

    Quaternion new_rotation = editor_state->old_entity->transform.rotation;

    // this will always intersect unless your FOV is >= 180 degrees
    if (intersects_plane) {
        Vec3 intersect_point = cursor_ray.origin + t*cursor_ray.direction;

        Vec3 center_to_intersect_point = intersect_point - center;
        Vec3 center_to_last_intersect_point = initial_hit - center;

        if (are_collinear(normalize(center_to_last_intersect_point), normalize(center_to_intersect_point))) {
            return new_rotation;
        }

        Vec3 out_vector = cross(editor_state->gizmo_transform_axis, center_to_last_intersect_point);
        
        real32 sign = 1.0f;
        if (dot(center_to_intersect_point, out_vector) < 0.0f) sign = -1.0f;

        real32 a = distance(center_to_intersect_point);
        real32 b = distance(center_to_last_intersect_point);
        real32 c = distance(intersect_point - initial_hit);

        real32 angle_delta_degrees = sign * cosine_law_degrees(a, b, c);

        Quaternion delta_from_start = make_quaternion(angle_delta_degrees, editor_state->gizmo_transform_axis);
        new_rotation = delta_from_start * new_rotation;
    }

    return new_rotation;
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

Vec3 get_gizmo_hit(Gizmo gizmo, Gizmo_Handle picked_handle, real32 pick_gizmo_t,
                   Ray cursor_ray, Vec3 transform_axis) {
    Vec3 gizmo_hit;

    if (is_rotation(picked_handle)) {
        real32 t;
        Plane plane = { dot(gizmo.transform.position, transform_axis), transform_axis };
        bool32 intersects_plane = ray_intersects_plane(cursor_ray, plane, &t);
        if (intersects_plane) {
            gizmo_hit = cursor_ray.origin + t*cursor_ray.direction;
        } else {
            gizmo_hit = cursor_ray.origin + pick_gizmo_t*cursor_ray.direction;;
        }
    } else {
        gizmo_hit = cursor_ray.origin + pick_gizmo_t*cursor_ray.direction;
    }

    return gizmo_hit;
}

void update_gizmo(Game_State *game_state) {
    Editor_State *editor_state = &game_state->editor_state;
    if (editor_state->selected_entity_id < 0) return;

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
    Asset_Manager *asset_manager = &game_state->asset_manager;

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

    if (just_pressed(controller_state->key_z)) {
        editor_state->show_wireframe = !editor_state->show_wireframe;
    }

    if (just_pressed(controller_state->key_x)) {
        if (editor_state->transform_mode == TRANSFORM_GLOBAL) {
            editor_state->transform_mode = TRANSFORM_LOCAL;
        } else {
            editor_state->transform_mode = TRANSFORM_GLOBAL;
        }

        if (editor_state->selected_gizmo_handle != GIZMO_HANDLE_NONE) {
            Entity *selected_entity = get_selected_entity(game_state);
            set_entity_transform(asset_manager, selected_entity, editor_state->old_entity->transform);
            //editor_state->gizmo_initial_hit =
        }
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
            int32 entity_id;
            bool32 picked = pick_entity(game_state, cursor_ray, &entity, &entity_id);
            
            if (picked) {
                if (selected_entity_changed(editor_state, entity_id, entity.type)) {
                    editor_state->last_selected_entity_type = editor_state->selected_entity_type;
                    editor_state->last_selected_entity_id = editor_state->selected_entity_id;

                    editor_state->selected_entity_type = entity.type;
                    editor_state->selected_entity_id = entity_id;

                    editor_state->gizmo.transform = entity.transform;

                    reset_entity_editors(editor_state);
                }
            } else {
                editor_state->selected_entity_id = -1;
            }
        }
    }

    update_gizmo(game_state);

    // gizmo picking
    if (editor_state->selected_entity_id >= 0 &&
        !ui_has_hot(ui_manager) &&
        !editor_state->selected_gizmo_handle) {

        real32 pick_gizmo_t;
        Gizmo_Handle picked_handle = pick_gizmo(game_state, cursor_ray, &pick_gizmo_t);
        if (controller_state->left_mouse.is_down && !controller_state->left_mouse.was_down) {
            if (picked_handle) {
                Entity *selected_entity = get_selected_entity(game_state);
                start_entity_change(editor_state, selected_entity);
#if 0
                editor_state->old_entity = allocate_and_copy_entity(editor_state->history.allocator_pointer,
                                                                    selected_entity);
#endif
                Vec3 global_transform_axis = get_gizmo_transform_axis(TRANSFORM_GLOBAL,
                                                                      picked_handle,
                                                                      selected_entity->transform);
                Vec3 local_transform_axis = get_gizmo_transform_axis(TRANSFORM_LOCAL,
                                                                     picked_handle,
                                                                     selected_entity->transform);
                editor_state->local_initial_gizmo_hit = get_gizmo_hit(editor_state->gizmo,
                                                                      picked_handle, pick_gizmo_t,
                                                                      cursor_ray, local_transform_axis);
                editor_state->global_initial_gizmo_hit = get_gizmo_hit(editor_state->gizmo,
                                                                       picked_handle, pick_gizmo_t,
                                                                       cursor_ray, global_transform_axis);
            }

            editor_state->selected_gizmo_handle = picked_handle;
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
        Entity *entity = get_selected_entity(game_state);
        if (controller_state->left_mouse.is_down) {
            editor_state->gizmo_transform_axis = get_gizmo_transform_axis(editor_state->transform_mode,
                                                                          editor_state->selected_gizmo_handle,
                                                                          editor_state->old_entity->transform);

            if (is_translation(editor_state->selected_gizmo_handle)) {
                Vec3 new_position = do_gizmo_translation(&render_state->camera, editor_state, cursor_ray);
                update_entity_position(asset_manager, entity, new_position);
                
            } else if (is_rotation(editor_state->selected_gizmo_handle)) {
                Quaternion new_rotation = do_gizmo_rotation(&render_state->camera,
                                                            editor_state, cursor_ray);
                update_entity_rotation(asset_manager, entity, new_rotation);
            } else if (is_scale(editor_state->selected_gizmo_handle)) {
                Vec3 new_scale = do_gizmo_scale(&render_state->camera, editor_state, cursor_ray);
                update_entity_scale(asset_manager, entity, new_scale);
            } else {
                assert(!"Should be unreachable.");
            }

            editor_state->gizmo.transform.position = entity->transform.position;
            editor_state->gizmo.transform.rotation = entity->transform.rotation;
        } else {
            editor_state->selected_gizmo_handle = GIZMO_HANDLE_NONE;
            finalize_entity_change(editor_state, &game_state->current_level, entity);
        }
    }

    update_gizmo(game_state);

#if 0
    if (editor_state->selected_entity_id >= 0) {
        Level *level = &game_state->current_level;
        Entity *entity = get_selected_entity(game_state);
        if (entity->type == ENTITY_NORMAL) {
            Normal_Entity *normal_entity = (Normal_Entity *) entity;
            if (normal_entity->collider.type == Collider_Type::CIRCLE) {
                Circle_Collider collider = normal_entity->collider.circle;
                real32 low_offset = 5.0f;
                Vec3 highest_point = make_vec3(0.0f, collider.center.y - low_offset, 0.0f);
                int32 triangle_index = -1;
                Vec3 triangle_normal;
                bool32 found_walkable_point = false;

                Hash_Table_Iterator<int32, Normal_Entity> iterator =
                    make_hash_table_iterator(level->normal_entity_table);
                Normal_Entity *e;
                while (e = get_next_value_pointer(&iterator)) {
                    if (e->is_walkable) {
                        Mesh *mesh = get_mesh_pointer(game_state, level, e->mesh_type, e->mesh_id);
                        Get_Walkable_Triangle_On_Mesh_Result result;
                        bool32 found_triangle = get_walkable_triangle_on_mesh(collider.center, collider.radius,
                                                                              mesh,
                                                                              e->transform,
                                                                              collider.center.y - low_offset,
                                                                              collider.center.y + 5.0f,
                                                                              &result);
                        if (found_triangle && (result.point.y > highest_point.y)) {
                            highest_point = result.point;
                            triangle_index = result.triangle_index;
                            triangle_normal = result.triangle_normal;
                            found_walkable_point = true;
                        }
                    }
                }

                if (found_walkable_point) {
                    add_debug_line(&game_state->debug_state, entity->transform.position, highest_point,
                                   make_vec4(1.0f, 1.0f, 0.0f, 1.0f));
                }
            }
        }
    }
#endif
}
