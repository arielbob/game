
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

    char *frames_string = string_format(frame_arena, "Frames: %d / %d",
                                        debugger_state->num_debug_frames, MAX_COLLISION_DEBUG_FRAMES);
    do_text(frames_string);
    ui_pop_widget();
    
    ui_pop_widget();
}
