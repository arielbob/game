void export_level(Allocator *allocator, Game_State *game_state, char *filename) {
    uint32 buffer_size = MEGABYTES(8); // should be a fine size
    String_Buffer working_buffer = make_string_buffer(allocator, buffer_size);

    Editor_State *editor_state = &game_state->editor_state;
    
    append_string(&working_buffer, ";; level info\n");
    append_string(&working_buffer, "level_name ");
    append_string(&working_buffer, "\"");
    append_string(&working_buffer, editor_state->level_name);
    append_string(&working_buffer, "\"");
    append_string(&working_buffer, "\n\n;; meshes\n");

    Hash_Table<int32, Mesh> mesh_table = game_state->mesh_table;
    for (int32 i = 0; i < mesh_table.max_entries; i++) {
        Hash_Table_Entry<int32, Mesh> entry = mesh_table.entries[i];
        if (entry.is_occupied) {
            append_string(&working_buffer, "mesh ");
            append_string(&working_buffer, "\"");
            append_string(&working_buffer, entry.value.name);
            append_string(&working_buffer, "\" ");
            append_string(&working_buffer, "\"");
            append_string(&working_buffer, entry.value.filename);
            append_string(&working_buffer, "\"");
            append_string(&working_buffer, "\n");
        }
    }
    
    bool32 write_result = platform_write_file(filename, working_buffer.contents, working_buffer.current_length, true);
    assert(write_result);
}
