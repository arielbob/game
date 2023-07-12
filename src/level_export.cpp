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

void append_string_property(String_Buffer *buffer, char *label, String string) {
    append_string(buffer, "%s ", label);
    append_string_add_quotes(buffer, string);
    append_string(buffer, "\n");
}

void append_vec3_property(String_Buffer *buffer, char *label, Vec3 v) {
    append_string(buffer, "%s ", label);
    append_string(buffer, "%f %f %f\n",
                  v.x, v.y, v.z);
}

void append_quaternion_property(String_Buffer *buffer, char *label, Quaternion q) {
    append_string(buffer, "%s ", label);
    append_string(buffer, "%f %f %f %f\n",
                  q.w, q.v.x, q.v.y, q.v.z);
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
        Mesh **mesh_table = asset_manager->mesh_table;
        for (int32 i = 0; i < NUM_MESH_BUCKETS; i++) {
            Mesh *current = mesh_table[i];
            
            while (current) {
                if (current->type == Mesh_Type::LEVEL) {
                    append_string(&working_buffer, "mesh ");
                    append_string_add_quotes(&working_buffer, current->name);
                    append_string(&working_buffer, " ");
                    append_string_add_quotes(&working_buffer, current->filename);
                    append_string(&working_buffer, "\n");
                }
                
                current = current->table_next;
            }
        }
    }
    append_string(&working_buffer, "}\n\n");

    append_string(&working_buffer, "textures {\n");
    {
        Texture **texture_table = asset_manager->texture_table;
        for (int32 i = 0; i < NUM_TEXTURE_BUCKETS; i++) {
            Texture *current = texture_table[i];
            
            while (current) {
                if (current->type == Texture_Type::LEVEL) {
                    append_string(&working_buffer, "texture ");
                    append_string_add_quotes(&working_buffer, current->name);
                    append_string(&working_buffer, " ");
                    append_string_add_quotes(&working_buffer, current->filename);
                    append_string(&working_buffer, "\n");
                }
                
                current = current->table_next;
            }
        }
    }
    append_string(&working_buffer, "}\n\n");
    
    int32 temp_buffer_size = 128;

    append_string(&working_buffer, "materials {\n");
    {
        Material **material_table = asset_manager->material_table;
        for (int32 i = 0; i < NUM_MATERIAL_BUCKETS; i++) {
            Material *current = material_table[i];
            
            while (current) {
                if (current->type != Material_Type::LEVEL) {
                    current = current->table_next;
                    continue;
                }

                append_string(&working_buffer, "material {\n");
                append_string(&working_buffer, "name ");
                append_string_add_quotes(&working_buffer, current->name);
                append_string(&working_buffer, "\n");

                if (current->flags & MATERIAL_USE_ALBEDO_TEXTURE) {
                    append_string(&working_buffer, "use_albedo_texture %d\n", 1);
                    append_string_property(&working_buffer, "albedo_texture", current->albedo_texture_name);
                }
                append_vec3_property(&working_buffer, "albedo_color", current->albedo_color);

                if (current->flags & MATERIAL_USE_METALNESS_TEXTURE) {
                    append_string(&working_buffer, "use_metalness_texture %d\n", 1);
                    append_string_property(&working_buffer, "metalness_texture", current->metalness_texture_name);
                }
                append_string(&working_buffer, "metalness %f\n", current->metalness);

                if (current->flags & MATERIAL_USE_ROUGHNESS_TEXTURE) {
                    append_string(&working_buffer, "use_roughness_texture %d\n", 1);
                    append_string_property(&working_buffer, "roughness_texture", current->roughness_texture_name);
                }
                append_string(&working_buffer, "roughness %f\n", current->roughness);
                append_string(&working_buffer, "}\n\n");
                
                current = current->table_next;
            }
        }
    }
    append_string(&working_buffer, "}\n\n");

    append_string(&working_buffer, "entities {\n");
    {
        Entity *current = level->entities;
        while (current) {
            append_string(&working_buffer, "entity {\n");
            append_vec3_property(&working_buffer, "position", current->transform.position);
            append_quaternion_property(&working_buffer, "rotation", current->transform.rotation);
            append_vec3_property(&working_buffer, "scale", current->transform.scale);

            if (current->flags & ENTITY_MESH) {
                Mesh *mesh = get_mesh(current->mesh_id);
                append_string_property(&working_buffer, "mesh", mesh->name);
            }

            if (current->flags & ENTITY_MATERIAL) {
                Material *material = get_material(current->material_id);
                append_string_property(&working_buffer, "material", material->name);
            }

            if (current->flags & ENTITY_LIGHT) {
                if (current->light_type == LIGHT_POINT) {
                    append_string(&working_buffer, "light_type point\n");
                    append_vec3_property(&working_buffer, "light_color", current->light_color);
                    append_string(&working_buffer, "falloff_start %f\n", current->falloff_start);
                    append_string(&working_buffer, "falloff_end %f\n", current->falloff_end);
                } else {
                    assert(!"Unhandled light type.");
                }
            }
            
            // TODO: export collider info
            
            append_string(&working_buffer, "}\n");
            
            current = current->next;
        }
    }
    append_string(&working_buffer, "}\n\n");

    bool32 write_result = platform_write_file(filename, working_buffer.contents, working_buffer.current_length, true);
    assert(write_result);

    end_region(m);
}
