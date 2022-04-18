void append_string_add_quotes(String_Buffer *buffer, String_Buffer string) {
    append_string(buffer, "\"");
    append_string(buffer, string);
    append_string(buffer, "\"");
}

void append_string_add_quotes(String_Buffer *buffer, char *string) {
    append_string(buffer, "\"");
    append_string(buffer, string);
    append_string(buffer, "\"");
}

void export_level(Allocator *allocator, Game_State *game_state, char *filename) {
    uint32 buffer_size = MEGABYTES(8); // should be a fine size
    String_Buffer working_buffer = make_string_buffer(allocator, buffer_size);

    Editor_State *editor_state = &game_state->editor_state;
    
    append_string(&working_buffer, ";; level info\n");
    append_string(&working_buffer, "level_name ");
    append_string_add_quotes(&working_buffer, editor_state->level_name);
    append_string(&working_buffer, "\n\n;; meshes\n");

    Hash_Table<int32, Mesh> mesh_table = game_state->mesh_table;
    for (int32 i = 0; i < mesh_table.max_entries; i++) {
        Hash_Table_Entry<int32, Mesh> entry = mesh_table.entries[i];
        if (entry.is_occupied) {
            append_string(&working_buffer, "mesh ");
            append_string_add_quotes(&working_buffer, entry.value.name);
            append_string(&working_buffer, " ");
            append_string_add_quotes(&working_buffer, entry.value.filename);
            append_string(&working_buffer, "\n");
        }
    }

    append_string(&working_buffer, "\n;; textures\n");
    Hash_Table<int32, Texture> texture_table = game_state->texture_table;
    for (int32 i = 0; i < texture_table.max_entries; i++) {
        Hash_Table_Entry<int32, Texture> entry = texture_table.entries[i];
        if (entry.is_occupied) {
            append_string(&working_buffer, "texture ");
            append_string_add_quotes(&working_buffer, entry.value.name);
            append_string(&working_buffer, " ");
            append_string_add_quotes(&working_buffer, entry.value.filename);
            append_string(&working_buffer, "\n");
        }
    }
    
    int32 temp_buffer_size = 128;

    append_string(&working_buffer, "\n;; materials\n");
    Hash_Table<int32, Material> material_table = game_state->material_table;
    for (int32 i = 0; i < material_table.max_entries; i++) {
        Hash_Table_Entry<int32, Material> entry = material_table.entries[i];
        if (entry.is_occupied) {
            Material material = entry.value;

            append_string(&working_buffer, "material ");
            append_string_add_quotes(&working_buffer, material.name);
            append_string(&working_buffer, "\n");
            
            if (material.texture_id >= 0) {
                append_string(&working_buffer, "texture ");
                Texture material_texture = get_texture(game_state, material.texture_id);
                append_string_add_quotes(&working_buffer, material_texture.name);
                append_string(&working_buffer, "\n");
            }

            Marker m = begin_region();

            append_string(&working_buffer, "gloss ");
            char *gloss_string = (char *) region_push(temp_buffer_size);
            string_format(gloss_string, temp_buffer_size, "%f", material.gloss);
            append_string(&working_buffer, gloss_string);
            append_string(&working_buffer, "\n");


            append_string(&working_buffer, "color_override ");
            char *color_override_string = (char *) region_push(temp_buffer_size);
            string_format(color_override_string, temp_buffer_size, "%f %f %f",
                          material.color_override.x, material.color_override.y, material.color_override.z);
            append_string(&working_buffer, color_override_string);
            append_string(&working_buffer, "\n");

            append_string(&working_buffer, "use_color_override ");
            append_string(&working_buffer, material.use_color_override ? "1" : "0");
            append_string(&working_buffer, "\n");

            end_region(m);

            append_string(&working_buffer, "\n");
        }
    }

    bool32 write_result = platform_write_file(filename, working_buffer.contents, working_buffer.current_length, true);
    assert(write_result);
}
