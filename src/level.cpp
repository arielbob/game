void export_level(Allocator *allocator, Game_State *game_state, char *filename) {
    uint32 buffer_size = MEGABYTES(8); // should be a fine size
    String_Buffer working_buffer = make_string_buffer(allocator, buffer_size);

    append_string(&working_buffer, "level_name");
    append_string(&working_buffer, "");
    //append_string(&working_buffer, game_state->
    
    //char test_string[] = "Hello, world!";
    bool32 write_result = platform_write_file(filename, working_buffer.contents, working_buffer.current_length, true);
    assert(write_result);
}
