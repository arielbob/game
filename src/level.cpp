#include "game.h"

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

void append_default_entity_info(Game_State *game_state, String_Buffer *buffer, Entity *entity) {
    append_string(buffer, "mesh ");
    Mesh mesh = get_mesh(game_state, entity->mesh_id);
    append_string_add_quotes(buffer, mesh.name);
    append_string(buffer, "\n");

    append_string(buffer, "material ");
    Material material = get_material(game_state, entity->material_id);
    append_string_add_quotes(buffer, material.name);
    append_string(buffer, "\n");

    int32 temp_buffer_size = 128;

    Marker m = begin_region();

    Transform transform = entity->transform;

    append_string(buffer, "position ");
    char *position_string = (char *) region_push(temp_buffer_size);
    string_format(position_string, temp_buffer_size, "%f %f %f",
                  transform.position.x, transform.position.y, transform.position.z);
    append_string(buffer, position_string);
    append_string(buffer, "\n");

    append_string(buffer, "rotation ");
    char *rotation_string = (char *) region_push(temp_buffer_size);
    string_format(rotation_string, temp_buffer_size, "%f %f %f %f",
                  transform.rotation.w, transform.rotation.v.x, transform.rotation.v.y, transform.rotation.v.z);
    append_string(buffer, rotation_string);
    append_string(buffer, "\n");

    append_string(buffer, "scale ");
    char *scale_string = (char *) region_push(temp_buffer_size);
    string_format(scale_string, temp_buffer_size, "%f %f %f",
                  transform.scale.x, transform.scale.y, transform.scale.z);
    append_string(buffer, scale_string);
    append_string(buffer, "\n");

    end_region(m);
}

void export_level(Allocator *allocator, Game_State *game_state, char *filename) {
    uint32 buffer_size = MEGABYTES(8); // should be a fine size
    String_Buffer working_buffer = make_string_buffer(allocator, buffer_size);

    Editor_State *editor_state = &game_state->editor_state;
    
    append_string(&working_buffer, "level info {\n");
    append_string(&working_buffer, "level_name ");
    append_string_add_quotes(&working_buffer, editor_state->level_name);
    append_string(&working_buffer, "\n");
    append_string(&working_buffer, "}\n\n");

    append_string(&working_buffer, "meshes {\n");
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
    append_string(&working_buffer, "}\n\n");

    append_string(&working_buffer, "textures {\n");
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
    append_string(&working_buffer, "}\n\n");
    
    int32 temp_buffer_size = 128;

    append_string(&working_buffer, "materials {\n");
    Hash_Table<int32, Material> material_table = game_state->material_table;
    int32 num_materials_added = 0;
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

            num_materials_added++;

            if (num_materials_added < material_table.num_entries) append_string(&working_buffer, "\n");
        }
    }
    append_string(&working_buffer, "}\n\n");

    append_string(&working_buffer, "entities {\n");

    // NORMAL ENTITIES
    for (int32 i = 0; i < game_state->num_normal_entities; i++) {
        Normal_Entity *entity = &game_state->normal_entities[i];

        append_string(&working_buffer, "type normal\n");
        append_default_entity_info(game_state, &working_buffer, (Entity *) entity);

        if (i < game_state->num_normal_entities - 1) append_string(&working_buffer, "\n");
    }

    if (game_state->num_point_lights > 0) append_string(&working_buffer, "\n");

    // POINT LIGHT ENTITIES
    for (int32 i = 0; i < game_state->num_point_lights; i++) {
        Point_Light_Entity *entity = &game_state->point_lights[i];

        append_string(&working_buffer, "type point_light\n");
        append_default_entity_info(game_state, &working_buffer, (Entity *) entity);

        Marker m = begin_region();

        append_string(&working_buffer, "light_color ");
        char *light_color_string = (char *) region_push(temp_buffer_size);
        string_format(light_color_string, temp_buffer_size, "%f %f %f",
                      entity->light_color.x, entity->light_color.y, entity->light_color.z);
        append_string(&working_buffer, light_color_string);
        append_string(&working_buffer, "\n");

        append_string(&working_buffer, "falloff_start ");
        char *falloff_start_string = (char *) region_push(temp_buffer_size);
        string_format(falloff_start_string, temp_buffer_size, "%f",
                      entity->falloff_start);
        append_string(&working_buffer, falloff_start_string);
        append_string(&working_buffer, "\n");

        append_string(&working_buffer, "falloff_end ");
        char *falloff_end_string = (char *) region_push(temp_buffer_size);
        string_format(falloff_end_string, temp_buffer_size, "%f",
                      entity->falloff_end);
        append_string(&working_buffer, falloff_end_string);
        append_string(&working_buffer, "\n");

        end_region(m);

        if (i < game_state->num_point_lights - 1) append_string(&working_buffer, "\n");
    }
    append_string(&working_buffer, "}\n");


    bool32 write_result = platform_write_file(filename, working_buffer.contents, working_buffer.current_length, true);
    assert(write_result);
}
