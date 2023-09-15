Collision_Debug_Frame *collision_debug_start_frame(Collision_Debug_State *state) {
    Collision_Debug_Frame *debug_frames = state->debug_frames;
    int32 *debug_frame_start_index = &state->debug_frame_start_index;
    int32 *num_debug_frames = &state->num_debug_frames;

    int32 new_frame_index = -1;
    if (*num_debug_frames >= MAX_COLLISION_DEBUG_FRAMES) {
        new_frame_index = ((*debug_frame_start_index + MAX_COLLISION_DEBUG_FRAMES) %
                           MAX_COLLISION_DEBUG_FRAMES);

        *debug_frame_start_index = (*debug_frame_start_index + 1) % MAX_COLLISION_DEBUG_FRAMES;

        *num_debug_frames = MAX_COLLISION_DEBUG_FRAMES;
    } else {
        new_frame_index = (*num_debug_frames)++;
    }
    
    Collision_Debug_Frame *new_frame = &debug_frames[new_frame_index];
    *new_frame = {};

    return new_frame;
}

void collision_debug_end_frame(Collision_Debug_State *state, Collision_Debug_Frame *frame, Vec3 position) {
    frame->player_position = position;
    state->current_frame = 0;
}

void collision_debug_add_intersection(Collision_Debug_Frame *frame, Collision_Debug_Intersection info) {
    assert(frame->num_intersections < MAX_FRAME_INTERSECTIONS);
    frame->intersections[frame->num_intersections++] = info;
}

void draw_collision_debugger() {
    UI_Window_Theme window_theme = DEFAULT_WINDOW_THEME;

    Editor_State *editor_state = &game_state->editor_state;
    Collision_Debug_State *debugger_state = &editor_state->collision_debug_state;

    window_theme.size_type = { UI_SIZE_FIT_CHILDREN, UI_SIZE_FIT_CHILDREN };

    push_window("Collision Debugger", window_theme,
                "collision-debugger-window", "collision-debugger-window-title-bar");

    UI_Container_Theme container_theme = {
        { 5.0f, 5.0f, 5.0f, 5.0f },
        DEFAULT_BOX_BACKGROUND,
        UI_POSITION_NONE,
        {},
        { UI_SIZE_FILL_REMAINING, UI_SIZE_FILL_REMAINING },
        {},
        UI_LAYOUT_VERTICAL
    };

    ui_push_container(container_theme, "collision-debugger-content");

    char *frames_string = string_format(frame_arena, "Current Frame: %d / %d",
                                        (debugger_state->num_debug_frames > 0) ?
                                        (debugger_state->current_frame + 1) : 0,
                                        debugger_state->num_debug_frames);
    do_text(frames_string);
    ui_y_pad(1.0f);

    UI_Theme row_theme = {};
    row_theme.size_type = { UI_SIZE_FIT_CHILDREN, UI_SIZE_FIT_CHILDREN };
    row_theme.semantic_size = { 0.0f, 0.0f };
    row_theme.layout_type = UI_LAYOUT_HORIZONTAL;

    UI_Button_Theme frame_button_theme = editor_button_theme;
    frame_button_theme.padding = { 10.0f, 10.0f };

    ui_y_pad(5.0f);

    if (debugger_state->num_debug_frames > 0) {
        UI_Text_Field_Slider_Result result = do_text_field_slider((real32) debugger_state->current_frame + 1, 1,
                                                                  (real32) debugger_state->num_debug_frames,
                                                                  editor_slider_theme,
                                                                  "frame-slider", "frame-slider-text");
        debugger_state->current_frame = (int32) result.value - 1;
    }
    
    ui_y_pad(1.0f);
    ui_push_widget("", row_theme);
    {
        if (do_text_button("<", frame_button_theme, "prev-frame")) {
            if (debugger_state->current_frame > 0) {
                debugger_state->current_frame--;
            }
        }
        ui_x_pad(1.0f);
        if (do_text_button(">", frame_button_theme, "next-frame")) {
            if (debugger_state->current_frame < debugger_state->num_debug_frames - 1) {
                debugger_state->current_frame++;
            }
        }
    } ui_pop_widget();

    
    if (debugger_state->num_debug_frames > 0) {
        ui_y_pad(10.0f);
        if (do_text_button(debugger_state->show_player_capsule ? "Hide Player Capsule" : "Show Player Capsule",
                           editor_button_theme, "show-player-capsule")) {
            debugger_state->show_player_capsule = !debugger_state->show_player_capsule;
        }
    }    
    
    ui_pop_widget(); // container
    ui_pop_widget(); // window
}
