#include "linked_list.h"
#include "editor.h"

void init_editor_level(Editor_State *editor_state, Editor_Level *editor_level) {
    *editor_level = {};
    make_and_init_linked_list(Entity *, &editor_level->entities, (Allocator *) &editor_state->entity_heap);
}

void init_editor(Arena_Allocator *editor_arena, Editor_State *editor_state, Display_Output display_output) {
    *editor_state = {};

    // we can't fill up the arena completely, i.e. if the arena is 2 megabytes, we can't just do two 1 MB
    // allocations since the arena needs space for alignment padding.
    uint32 entity_heap_size = MEGABYTES(64);
    uint32 history_heap_size = MEGABYTES(64);
    uint32 general_heap_size = MEGABYTES(128) - 8*3; // padding will be at most 8 bytes * 3

    void *entity_heap_base = arena_push(editor_arena, entity_heap_size, false);
    editor_state->entity_heap = make_heap_allocator(entity_heap_base, entity_heap_size);

    void *history_heap_base = arena_push(editor_arena, history_heap_size, false);
    editor_state->history_heap = make_heap_allocator(history_heap_base, entity_heap_size);

    void *general_heap_base = arena_push(editor_arena, general_heap_size, false);
    editor_state->general_heap = make_heap_allocator(general_heap_base, entity_heap_size);

    init_camera(&editor_state->camera, &display_output);
    init_editor_level(editor_state, &editor_state->level);

    editor_state->asset_manager = make_asset_manager((Allocator *) &editor_state->general_heap);
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    load_default_assets(asset_manager);

    load_font(asset_manager, "c:/windows/fonts/times.ttf", "times32", 32.0f, 512, 512);
    load_font(asset_manager, "c:/windows/fonts/times.ttf", "times24", 24.0f, 512, 512);

    load_font(asset_manager, "c:/windows/fonts/courbd.ttf", "courier24b", 24.0f, 512, 512);

    load_font(asset_manager, "c:/windows/fonts/calibri.ttf", "calibri14", 14.0f, 512, 512);
    load_font(asset_manager, "c:/windows/fonts/calibrib.ttf", "calibri14b", 14.0f, 512, 512);
    load_font(asset_manager, "c:/windows/fonts/calibrib.ttf", "calibri24b", 24.0f, 512, 512);

    load_font(asset_manager, "c:/windows/fonts/lucon.ttf", "lucidaconsole18", 18.0f, 512, 512);
}

void unload_level(Editor_State *editor_state) {
    // TODO: set assets to be unloaded
    // TODO: deallocate the entities list
}

void load_level(Editor_State *editor_state, Level_Info *level_info) {
    Asset_Manager *asset_manager = &editor_state->asset_manager;
    load_level_assets(asset_manager, level_info);

    Editor_Level *level = &editor_state->level;
    level->name = copy((Allocator *) &editor_state->general_heap, level_info->name);

    Allocator *entity_allocator = (Allocator *) &editor_state->entity_heap;

    // NOTE: if we ever add allocated members to entities, we have to change these to use a copy procedure
    FOR_LIST_NODES(Normal_Entity_Info, level_info->normal_entities) {
        Normal_Entity_Info info = current_node->value;
        Normal_Entity entity = info.entity;

        if (info.flags & HAS_MESH) {
            entity.mesh_id = get_mesh_id_by_name(asset_manager, info.mesh_name);
        }
        if (info.flags & HAS_MATERIAL) {
            entity.material_id = get_material_id_by_name(asset_manager, info.material_name);
        }

        Normal_Entity *e = (Normal_Entity *) allocate(entity_allocator, sizeof(Normal_Entity));
        *e = entity;
        add(&level->entities, (Entity *) e);
    }

    FOR_LIST_NODES(Point_Light_Entity_Info, level_info->point_light_entities) {
        Point_Light_Entity entity = current_node->value.entity;
        Point_Light_Entity *e = (Point_Light_Entity *) allocate(entity_allocator, sizeof(Point_Light_Entity));
        *e = entity;
        add(&level->entities, (Entity *) e);
    }
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

            load_level(editor_state, &level_info);
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

void update_editor_camera(Editor_State *editor_state, Controller_State *controller_state,
                          bool32 has_focus, bool32 should_move,
                          real32 dt) {
    Camera *camera = &editor_state->camera;
    bool32 use_freecam = editor_state->use_freecam;
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

void update_editor(Game_State *game_state, Controller_State *controller_state, real32 dt) {
    UI_Manager *ui_manager = &game_state->ui_manager;
    Editor_State *editor_state = &game_state->editor_state;
    Render_State *render_state = &game_state->render_state;
    Display_Output *display_output = &game_state->render_state.display_output;
    Asset_Manager *asset_manager = &editor_state->asset_manager;

    if (just_pressed(controller_state->key_tab) && !has_focus(ui_manager)) {
        editor_state->use_freecam = !editor_state->use_freecam;
        platform_set_cursor_visible(!editor_state->use_freecam);
    }
    
    bool32 camera_should_move = editor_state->use_freecam && !has_focus(ui_manager);
    update_editor_camera(editor_state, controller_state,
                         platform_window_has_focus(), camera_should_move, dt);
    update_render_state(render_state, editor_state->camera);
}

void draw_editor(Game_State *game_state, Controller_State *controller_state) {
    Render_State *render_state = &game_state->render_state;
    Editor_State *editor_state = &game_state->editor_state;
    update_render_state(render_state, editor_state->camera);

    draw_level_box(&game_state->ui_manager, editor_state,
                   controller_state,
                   render_state->display_output.width - 198.0f - 1.0f, 1.0f);
}
