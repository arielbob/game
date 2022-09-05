#include "level.h"

void append_string_add_quotes(String_Buffer *buffer, String string) {
    append_string(buffer, "\"");
    append_string(buffer, string);
    append_string(buffer, "\"");
}

void append_string_add_quotes(String_Buffer *buffer, char *string) {
    append_string(buffer, "\"");
    append_string(buffer, string);
    append_string(buffer, "\"");
}

void append_default_entity_info(Level *level, String_Buffer *buffer, Entity *entity) {
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

void export_level(Level *level, char *filename) {
    Marker m = begin_region();

    uint32 buffer_size = MEGABYTES(8); // should be a fine size
    String_Buffer working_buffer = make_string_buffer(temp_region, buffer_size);

    append_string(&working_buffer, "level_info {\n");
    append_string(&working_buffer, "level_name ");
    append_string_add_quotes(&working_buffer, level->name);
    append_string(&working_buffer, "\n");
    append_string(&working_buffer, "}\n\n");

    append_string(&working_buffer, "meshes {\n");
    {
        FOR_VALUE_POINTERS(int32, Mesh, asset_manager->mesh_table) {
            if (value->type == Mesh_Type::LEVEL) {
                append_string(&working_buffer, "mesh ");
                append_string_add_quotes(&working_buffer, value->name);
                append_string(&working_buffer, " ");
                append_string_add_quotes(&working_buffer, value->filename);
                append_string(&working_buffer, "\n");
            }
        }
    }
    append_string(&working_buffer, "}\n\n");

    append_string(&working_buffer, "textures {\n");
    {
        FOR_VALUE_POINTERS(int32, Texture, asset_manager->texture_table) {
            append_string(&working_buffer, "texture ");
            append_string_add_quotes(&working_buffer, value->name);
            append_string(&working_buffer, " ");
            append_string_add_quotes(&working_buffer, value->filename);
            append_string(&working_buffer, "\n");
        }
    }
    append_string(&working_buffer, "}\n\n");
    
    int32 temp_buffer_size = 128;

    append_string(&working_buffer, "materials {\n");
    {
        int32 num_materials_added = 0;
        FOR_VALUE_POINTERS(int32, Material, asset_manager->material_table) {
            Material *material = value;
            append_string(&working_buffer, "material ");
            append_string_add_quotes(&working_buffer, material->name);
            append_string(&working_buffer, "\n");
            
            if (material->texture_id >= 0) {
                append_string(&working_buffer, "texture ");
                Texture material_texture = get_texture(asset_manager, material->texture_id);
                append_string_add_quotes(&working_buffer, material_texture.name);
                append_string(&working_buffer, "\n");
            }

            Marker m2 = begin_region();

            append_string(&working_buffer, "gloss ");
            char *gloss_string = (char *) region_push(temp_buffer_size);
            string_format(gloss_string, temp_buffer_size, "%f", material->gloss);
            append_string(&working_buffer, gloss_string);
            append_string(&working_buffer, "\n");

            append_string(&working_buffer, "color_override ");
            char *color_override_string = (char *) region_push(temp_buffer_size);
            string_format(color_override_string, temp_buffer_size, "%f %f %f",
                          material->color_override.x, material->color_override.y, material->color_override.z);
            append_string(&working_buffer, color_override_string);
            append_string(&working_buffer, "\n");

            append_string(&working_buffer, "use_color_override ");
            append_string(&working_buffer, material->use_color_override ? "1" : "0");
            append_string(&working_buffer, "\n");

            end_region(m2);

            num_materials_added++;

            if (num_materials_added < asset_manager->material_table.num_entries) append_string(&working_buffer, "\n");
        }
    }
    append_string(&working_buffer, "}\n\n");

    append_string(&working_buffer, "entities {\n");


    Linked_List<Normal_Entity *> normal_entities;
    Linked_List<Point_Light_Entity *> point_light_entities;

    gather_entities_by_type(temp_region, level,
                            &normal_entities, &point_light_entities);
    
    // NORMAL ENTITIES
    FOR_LIST_NODES(Normal_Entity *, normal_entities) {
        Normal_Entity *entity = current_node->value;

        append_string(&working_buffer, "type normal\n");
        append_default_entity_info(level, &working_buffer, (Entity *) entity);

        Mesh mesh = get_mesh(asset_manager, entity->mesh_id);

        if (mesh.type == Mesh_Type::PRIMITIVE) {
            append_string(&working_buffer, "mesh_primitive ");
        } else {
            append_string(&working_buffer, "mesh ");
        }
        append_string_add_quotes(&working_buffer, mesh.name);
        append_string(&working_buffer, "\n");

        if (entity->material_id >= 0) {
            append_string(&working_buffer, "material ");
            Material material = get_material(asset_manager, entity->material_id);
            append_string_add_quotes(&working_buffer, material.name);
            append_string(&working_buffer, "\n");
        }
            
        append_string(&working_buffer, "is_walkable ");
        append_string(&working_buffer, entity->is_walkable ? "1" : "0");
        append_string(&working_buffer, "\n");

        if (!is_last(&normal_entities, current_node)) append_string(&working_buffer, "\n");
    }

    if (normal_entities.num_entries > 0) append_string(&working_buffer, "\n");

    // POINT LIGHT ENTITIES
    FOR_LIST_NODES(Point_Light_Entity *, point_light_entities) {
        Point_Light_Entity *entity = current_node->value;

        append_string(&working_buffer, "type point_light\n");
        append_default_entity_info(level, &working_buffer, (Entity *) entity);

        Marker m2 = begin_region();

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

        end_region(m2);

        if (!is_last(&point_light_entities, current_node)) append_string(&working_buffer, "\n");
    }

    append_string(&working_buffer, "}\n");

    bool32 write_result = platform_write_file(filename, working_buffer.contents, working_buffer.current_length, true);
    assert(write_result);

    end_region(m);
}
