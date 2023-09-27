void reset_collision_debugger_state(Collision_Debug_State *state) {
    bool32 show_capsule = state->show_player_capsule;
    *state = {};
    state->show_player_capsule = show_capsule;
}

Collision_Debug_Frame *get_current_frame(Collision_Debug_State *state) {
    int32 frame_index = (state->current_frame + state->debug_frame_start_index)
        % MAX_COLLISION_DEBUG_FRAMES;
    Collision_Debug_Frame *frame = &state->debug_frames[frame_index];

    return frame;
}

void collision_debug_prev_frame(Collision_Debug_State *state) {
    if (state->current_frame > 0) {
        state->current_frame--;
        state->current_subframe = 0;
    }
}

void collision_debug_next_frame(Collision_Debug_State *state) {
    if (state->current_frame < state->num_debug_frames - 1) {
        state->current_frame++;
        state->current_subframe = 0;
    }
}

void collision_debug_prev_subframe(Collision_Debug_State *state) {
    if (state->current_subframe > 0) {
        state->current_subframe--;
    }
}

void collision_debug_next_subframe(Collision_Debug_State *state) {
    Collision_Debug_Frame *current_frame = get_current_frame(state);
    if (state->current_subframe < current_frame->num_subframes - 1) {
        state->current_subframe++;
    }
}

void collision_debug_log_subframe(Collision_Debug_Frame *frame,
                                  Collision_Debug_Subframe subframe) {
    assert(frame->num_subframes < MAX_COLLISION_DEBUG_FRAME_SUBFRAMES);
    frame->subframes[frame->num_subframes++] = subframe;
}

Collision_Debug_Frame *collision_debug_start_frame(Collision_Debug_State *state, Vec3 initial_position) {
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
    Collision_Debug_Subframe end_subframe = { COLLISION_SUBFRAME_POSITION };
    end_subframe.position = { position };
    collision_debug_log_subframe(frame, end_subframe);
    
    state->current_frame = 0;
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
        if ((result.value - 1) != debugger_state->current_frame) {
            debugger_state->current_subframe = 0;
        }
        debugger_state->current_frame = (int32) result.value - 1;
    }
    
    ui_y_pad(1.0f);
    ui_push_widget("", row_theme);
    {
        if (do_text_button("<", frame_button_theme, "prev-frame")) {
            collision_debug_prev_frame(debugger_state);
        }
        ui_x_pad(1.0f);
        if (do_text_button(">", frame_button_theme, "next-frame")) {
            collision_debug_next_frame(debugger_state);
        }
    } ui_pop_widget();

    Collision_Debug_Frame *current_frame = get_current_frame(debugger_state);
    if (current_frame->num_subframes > 0) {
        ui_push_container(container_theme, "collision-debugger-inner-frame-content");
        {
            char *subframes_string = string_format(frame_arena, "Subframe: %d / %d",
                                                   debugger_state->current_subframe + 1,
                                                   current_frame->num_subframes);
            do_text(subframes_string);

            ui_y_pad(1.0f);
            ui_push_widget("", row_theme);
            {
                if (do_text_button("<", frame_button_theme, "prev-inner-frame")) {
                    collision_debug_prev_subframe(debugger_state);
                }
                ui_x_pad(1.0f);
                if (do_text_button(">", frame_button_theme, "next-inner-frame")) {
                    collision_debug_next_subframe(debugger_state);
                }
            } ui_pop_widget();

        } ui_pop_widget();
    }
        
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
