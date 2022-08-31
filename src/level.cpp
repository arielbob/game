#include "linked_list.h"
#include "level.h"

#if 0
void init_level_info(Allocator *temp_allocator, Level_Info *level_info) {
    *level_info = {};

    make_and_init_linked_list(Normal_Entity_Info,      &level_info->normal_entities,      temp_allocator);
    make_and_init_linked_list(Point_Light_Entity_Info, &level_info->point_light_entities, temp_allocator);
    make_and_init_linked_list(Mesh_Info,               &level_info->meshes,               temp_allocator);
    make_and_init_linked_list(Texture_Info,            &level_info->textures,             temp_allocator);
    make_and_init_linked_list(Material_Info,           &level_info->materials,            temp_allocator);
}
#endif

// gather entities by type
void gather_entities_by_type(Allocator *allocator,
                             Editor_Level *level,
                             Linked_List<Normal_Entity *> *normal_entities,
                             Linked_List<Point_Light_Entity *> *point_light_entities) {
    make_and_init_linked_list(Point_Light_Entity *, point_light_entities, allocator);
    make_and_init_linked_list(Normal_Entity *, normal_entities, allocator);

    FOR_LIST_NODES(Entity *, level->entities) {
        Entity *entity = current_node->value;
        switch (entity->type) {
            case ENTITY_NORMAL: {
                add(normal_entities, (Normal_Entity *) entity);
            } break;
            case ENTITY_POINT_LIGHT: {
                add(point_light_entities, (Point_Light_Entity *) entity);
            } break;
            default: {
                assert(!"Unhandled entity type.");
            }
        }
    }

}

bool32 mesh_name_exists(Level_Info *level_info, String mesh_name) {
    FOR_LIST_NODES(Mesh_Info, level_info->meshes) {
        Mesh_Info info = current_node->value;
        if (string_equals(info.name, mesh_name)) {
            return true;
        }
    }
    return false;
}

bool32 texture_name_exists(Level_Info *level_info, String texture_name) {
    FOR_LIST_NODES(Texture_Info, level_info->textures) {
        Texture_Info info = current_node->value;
        if (string_equals(info.name, texture_name)) {
            return true;
        }
    }
    return false;
}

bool32 material_name_exists(Level_Info *level_info, String material_name) {
    FOR_LIST_NODES(Material_Info, level_info->materials) {
        Material_Info info = current_node->value;
        if (string_equals(info.name, material_name)) {
            return true;
        }
    }
    return false;
}

void level_info_add_mesh(Level_Info *level_info, String mesh_name, String mesh_filename) {
    assert(level_info->num_meshes < MAX_LEVEL_INFO_ARRAY_SIZE);
    level_info->meshes[level_info->num_meshes++] = { mesh_name, mesh_filename };
}

void level_info_add_texture(Level_Info *level_info, String texture_name, String texture_filename) {
    assert(level_info->num_textures < MAX_LEVEL_INFO_ARRAY_SIZE);
    level_info->textures[level_info->num_textures++] = { texture_name, texture_filename };
}

void level_info_add_material(Level_Info *level_info, Material_Info material) {
    assert(level_info->num_materials < MAX_LEVEL_INFO_ARRAY_SIZE);
    level_info->materials[level_info->num_materials++] = material;
}

Level_Loader::Token Level_Loader::parse_mesh(Tokenizer *tokenizer, Level_Info *level_info) {
    Token token = get_token(tokenizer);
    if (token.type == KEYWORD &&
        string_equals(token.string, "mesh")) {
    } else if (token.type == CLOSE_BRACKET) {
        state = WAIT_FOR_TEXTURES_BLOCK_NAME;
    } else {
        assert(!"Expected mesh keyword or closing bracket.");
    }
}

inline Level_Loader::Token Level_Loader::make_token(Token_Type type, char *contents, int32 length) {
    Token token = {
        type,
        make_string(contents, length)
    };
    return token;
}

Level_Loader::Token Level_Loader::get_token(Tokenizer *tokenizer) {
    Token token = {};

    do {
        consume_leading_whitespace(tokenizer);

        if (is_end(tokenizer)) {
            token = make_token(END, NULL, 0);
            return token;
        }

        char c = *tokenizer->current;

        // it's fine to not use tokenizer_equals here since we've already checked for is_end and we're only
        // comparing against single characters
        if (is_digit(c) || (c == '-') || (c == '.')) {
            uint32 start = tokenizer->index;
        
            bool32 has_period = false;
            bool32 is_negative = false;
            if (c == '.') {
                has_period = true;
            } else if (c == '-') {
                is_negative = true;
            }

            increment_tokenizer(tokenizer);
        
            while (!is_end(tokenizer) && !is_whitespace(tokenizer)) {
                if (!is_digit(*tokenizer->current) &&
                    *tokenizer->current != '.') {
                    assert(!"Expected digit or period");
                }
            
                if (*tokenizer->current == '.') {
                    if (!has_period) {
                        has_period = true;
                    } else {
                        assert(!"More than one period in number");
                    }
                }

                if (*tokenizer->current == '-') {
                    assert(!"Negatives can only be at the start of a number");
                }

                increment_tokenizer(tokenizer);
            }
        
            uint32 length = tokenizer->index - start;

            Token_Type type;
            if (has_period) {
                type = REAL;
            } else {
                type = INTEGER;
            }

            token = make_token(type, &tokenizer->contents[start], length);
        } else if (tokenizer_equals(tokenizer, ";;")) {
            increment_tokenizer(tokenizer, 2);
            int32 start = tokenizer->index;
        
            // it is necessary that we check both is_end and is_line_end, since is_line_end will return false
            // if we hit the end, and so we'll be stuck in an infinite loop.
            while (!is_end(tokenizer) &&
                   !is_line_end(tokenizer)) {
                increment_tokenizer(tokenizer);
            }

            int32 length = tokenizer->index - start;
            token = make_token(COMMENT, &tokenizer->contents[start], length);
        } else if (is_letter(*tokenizer->current)) {
            uint32 start = tokenizer->index;

            increment_tokenizer(tokenizer);
        
            while (!is_end(tokenizer) &&
                   !is_whitespace(*tokenizer->current)) {

                char current_char = *tokenizer->current;
                if (!(is_letter(current_char) || current_char == '_')) {
                    assert(!"Keywords can only contain letters and underscores.");
                }

                increment_tokenizer(tokenizer);
            }

            int32 length = tokenizer->index - start;
            token = make_token(KEYWORD, &tokenizer->contents[start], length);
        } else if (*tokenizer->current == '"') {
            increment_tokenizer(tokenizer);
            // set start after we increment tokenizer so that we don't include the quote in the token string.
            int32 start = tokenizer->index;

            while (!is_end(tokenizer) &&
                   !(*tokenizer->current == '"')) {
                increment_tokenizer(tokenizer);
            }

            if (*tokenizer->current != '"') {
                assert(!"Expected a closing quote.");
            } else {
                int32 length = tokenizer->index - start;
                increment_tokenizer(tokenizer);
                token = make_token(STRING, &tokenizer->contents[start], length);
            }
        } else if (*tokenizer->current == '{') {
            int32 start = tokenizer->index;
            increment_tokenizer(tokenizer);
            int32 length = tokenizer->index - start;
            token = make_token(OPEN_BRACKET, &tokenizer->contents[start], length);
        } else if (*tokenizer->current == '}') {
            int32 start = tokenizer->index;
            increment_tokenizer(tokenizer);
            int32 length = tokenizer->index - start;
            token = make_token(CLOSE_BRACKET, &tokenizer->contents[start], length);
        } else {
            assert(!"Token type not recognized.");
        }
    } while (token.type != COMMENT);

    return token;
}

inline bool32 level_parse_error(char **error_out, char *error_string) {
    *error_out = error_string;
    return false;
}

bool32 level_parse_vec3(Tokenizer *tokenizer, Vec3 *result, char **error) {
    Token token;

    for (int i = 0; i < 3; i++) {
        token = get_token(tokenizer);
        if (token.type != REAL || token.type != INTEGER) {
            return level_parse_error(error, "Expected number.");
        } else {
            real32 num;
            bool32 real32_parse_result = string_to_real32(token.string, &num);

            if (real32_parse_result) {
                (*result)[i] = num;
            } else {
                return level_parse_error(error, "Invalid number in Vec3.");
            }
        }
    }

    return true;
}

bool32 level_parse_quaternion(Tokenizer *tokenizer, Quaternion *result, char **error) {
    Token token = get_token(tokenizer);

    if (token.type != REAL || token.type != INTEGER) {
        return level_parse_error(error, "Expected number for w value (first value) of quaternion.");
    }

    real32 num;
    bool32 real32_parse_result = string_to_real32(token.string, &num);
    if (real32_parse_result) {
        result->w = num;
    } else {
        return level_parse_error(error, "Invalid number for q value (first value) of quaternion.");
    }
    
    for (int i = 0; i < 3; i++) {
        token = get_token(tokenizer);
        if (token.type != REAL || token.type != INTEGER) {
            return level_parse_error(error, "Expected number in quaternion vector portion.");
        } else {
            real32 num;
            bool32 real32_parse_result = string_to_real32(token.string, &num);

            if (real32_parse_result) {
                result->v[i] = num;
            } else {
                return level_parse_error(error, "Invalid number in quaternion vector portion.");
            }
        }
    }

    return true;
}

bool32 parse_level_info_block(Allocator *temp_allocator, Tokenizer *tokenizer,
                              Level_Info *level_info, char **error) {
    Token token = get_token(tokenizer);

    if (!(token.type == KEYWORD && string_equals(token.string, "level_info"))) {
        return level_parse_error(error, "Expected level_info keyword.");
    }

    token = get_token(tokenizer);
    
    if (token.type != OPEN_BRACKET) {
        return level_parse_error(error, "Expected open bracket for level_info block.");
    }

    token = get_token(tokenizer);

    if (!(token.type == KEYWORD && string_equals(token.string, "level_name"))) {
        return level_parse_error(error, "Expected level_name keyword.");
    }

    token = get_token(tokenizer);

    if (token.type != STRING) {
        return level_parse_error(error, "Expected level name to be a string.");
    }

    if (token.string.length == 0) {
        return level_parse_error(error, "Level name cannot be empty.");
    }

    level_info.level_name = token.string;

    if (token.type != CLOSE_BRACKET) {
        return level_parse_error(error, "Expected close bracket for level_info_block");
    }

    return true;    
}

bool32 parse_meshes_block(Allocator *temp_allocator, Tokenizer *tokenizer,
                             Level_Info *level_info, char **error) {
    Token token = get_token(tokenizer);

    if (!(token.type == KEYWORD && string_equals(token.string, "meshes"))) {
        return level_parse_error(error, "Expected meshes keyword.");
    }

    token = get_token(tokenizer);
    
    if (token.type != OPEN_BRACKET) {
        return level_parse_error(error, "Expected open bracket for meshes block.");
    }

    token = get_token(tokenizer);

    while (token.type == KEYWORD && token.string == "mesh") {
        String mesh_name, mesh_filename;
        
        if (token.type != STRING) {
            return level_parse_error(error, "Expected string for mesh name.");
        }

        mesh_name = token.string;
        token = get_token(tokenizer);
        
        if (token.type != STRING) {
            return level_parse_error(error, "Expected string for mesh filename.");
        }

        mesh_filename = token.string;
        token = get_token(tokenizer);

        level_info_add_mesh(level_info, mesh_name, mesh_filename);
    }

    if (token.type != CLOSE_BRACKET) {
        return level_parse_error(error, "Expected close bracket for meshes block.");
    }
    
    return true;
    
}

bool32 parse_textures_block(Allocator *temp_allocator, Tokenizer *tokenizer,
                            Level_Info *level_info, char **error) {
    Token token = get_token(tokenizer);

    if (!(token.type == KEYWORD && string_equals(token.string, "textures"))) {
        return level_parse_error(error, "Expected textures keyword.");
    }

    token = get_token(tokenizer);
    
    if (token.type != OPEN_BRACKET) {
        return level_parse_error(error, "Expected open bracket for textures block.");
    }

    token = get_token(tokenizer);

    while (token.type == KEYWORD && token.string == "texture") {
        String texture_name, texture_filename;
        
        if (token.type != STRING) {
            return level_parse_error(error, "Expected string for texture name.");
        }

        texture_name = token.string;
        token = get_token(tokenizer);
        
        if (token.type != STRING) {
            return level_parse_error(error, "Expected string for texture filename.");
        }

        texture_filename = token.string;
        token = get_token(tokenizer);

        level_info_add_texture(level_info, texture_name, texture_filename);
    }

    if (token.type != CLOSE_BRACKET) {
        return level_parse_error(error, "Expected close bracket for textures block.");
    }
    
    return true;
    
}

// this should return false when it hits the closing bracket of the materials block
bool32 parse_material(Allocator *temp_allocator, Tokenizer *tokenizer,
                      Material_Info *material_info_out, char **error) {
    Token token = get_token(tokenizer);

    if (token.type == CLOSE_BRACKET) {
        return true;
    }
    
    if (!(token.type == KEYWORD && string_equals(token.string, "material"))) {
        return level_parse_error(error, "Expected material keyword or close bracket.");
    }

    token = get_token(tokenizer);
    
    if (token.type != OPEN_BRACKET) {
        return level_parse_error(error, "Expected open bracket for material block.");
    }

    token = get_token(tokenizer);

    Material_Info material_info;

    // set defaults
    material_info.flags = 0;
    material_info.albedo_color = make_vec3(1.0f, 0.0f, 0.0f);
    material_info.metalness = 0.5f;
    material_info.roughness = 0.5f;
    
    if (token.type == KEYWORD) {
        if (string_equals(token.string, "name")) {
            token = get_token(tokenizer);

            if (token.type == STRING) {
                if (!is_empty(token.string)) {
                    material_info.name = token.string;
                } else {
                    return level_parse_error(error, "Material name cannot be empty.");
                }
            } else {
                return level_parse_error(error, "Expected string for material name.");
            }
        } else {
            return level_parse_error(error, "Expected name keyword.");
        }
    } else {
        return level_parse_error(error, "Expected name keyword.");
    }

    token = get_token(tokenizer);
    
    // we accept duplicates of parameters and just set the material's parameter to the last one read
    // (except for the name parameter)
    do {
        if (string_equals(token.string, "name")) {
            return level_parse_error(error, "Material name already set.");
        } else if (string_equals(token.string, "use_albedo_texture")) {
            token = get_token(tokenizer);
            
            if (token.type == INTEGER) {
                uint32 result;
                bool32 parse_result = ascii_to_uint32(token.string, &result);
                if (parse_result) {
                    material_info.use_albedo_texture = result;
                } else {
                    return level_parse_error(error, "Expected unsigned integer for use_albedo_texture.");
                }
            } else {
                return level_parse_error(error, "Expected unsigned integer for use_albedo_texture.");
            }
        } else if (string_equals(token.string, "albedo_texture")) {
            token = get_token(tokenizer);
            
            String result;

            if (token.type == STRING) {
                if (!is_empty(token.string)) {
                    material_info.albedo_texture_name = token.string;
                } else {
                    return level_parse_error(error, "Albedo texture name cannot be empty.");
                }
            } else {
                return level_parse_error(error, "Expected string for albedo texture name.");
            }
        } else if (string_equals(token.string, "albedo_color")) {
            Vec3 result;

            bool32 parse_result = level_parse_vec3(tokenizer, &result, error);
            if (parse_result) {
                material_info.albedo_color = result;
            } else {
                // error message gets set by level_parse_vec3()
                return false;
            }
        } else if (string_equals(token.string, "use_metalness_texture")) {
            token = get_token(tokenizer);
            
            if (token.type == INTEGER) {
                uint32 result;
                bool32 parse_result = ascii_to_uint32(token.string, &result);
                if (parse_result) {
                    material_info.use_metalness_texture = result;
                } else {
                    return level_parse_error(error, "Expected unsigned integer for use_metalness_texture.");
                }
            } else {
                return level_parse_error(error, "Expected unsigned integer for use_metalness_texture.");
            }
        } else if (string_equals(token.string, "metalness_texture")) {
            token = get_token(tokenizer);
            
            String result;

            if (token.type == STRING) {
                if (!is_empty(token.string)) {
                    material_info.metalness_texture_name = token.string;
                } else {
                    return level_parse_error(error, "Metalness texture name cannot be empty.");
                }
            } else {
                return level_parse_error(error, "Expected string for metalness texture name.");
            }
        } else if (string_equals(token.string, "metalness")) {
            token = get_token(tokenizer);

            if (token.type == REAL || token.type == INTEGER) {
                real32 result;
                bool32 parse_result = string_to_real32(token.string, &result);

                if (parse_result) {
                    material_info.metalness = result;
                } else {
                    return level_parse_error("Invalid number for metalness value.");
                }
            } else {
                return level_parse_error("Expected number for metalness value.");
            }
        } else if (string_equals(token.string, "use_roughness_texture")) {
            token = get_token(tokenizer);
            
            if (token.type == INTEGER) {
                uint32 result;
                bool32 parse_result = ascii_to_uint32(token.string, &result);
                if (parse_result) {
                    material_info.use_roughness_texture = result;
                } else {
                    return level_parse_error(error, "Expected unsigned integer for use_roughness_texture.");
                }
            } else {
                return level_parse_error(error, "Expected unsigned integer for use_roughness_texture.");
            }
        } else if (string_equals(token.string, "roughness_texture")) {
            token = get_token(tokenizer);
            
            String result;

            if (token.type == STRING) {
                if (!is_empty(token.string)) {
                    material_info.roughness_texture_name = token.string;
                } else {
                    return level_parse_error(error, "Roughness texture name cannot be empty.");
                }
            } else {
                return level_parse_error(error, "Expected string for roughness texture name.");
            }
        } else if (string_equals(token.string, "roughness")) {
            token = get_token(tokenizer);

            if (token.type == REAL || token.type == INTEGER) {
                real32 result;
                bool32 parse_result = string_to_real32(token.string, &result);

                if (parse_result) {
                    material_info.roughness = result;
                } else {
                    return level_parse_error("Invalid number for roughness value.");
                }
            } else {
                return level_parse_error("Expected number for roughness value.");
            }
        } else {
            *error = "Unrecognized material property."
        }

        token = get_token(tokenizer);
    } while (token.type == KEYWORD);

    if (token.type != KEYWORD && token.type != CLOSE_BRACKET) {
        return level_parse_error(error, "Expected keyword for a material property or close bracket for material.");
    }
    
    *material_info_out = material_info;
    return true;
}

bool32 parse_materials_block(Allocator *temp_allocator, Tokenizer *tokenizer,
                             Level_Info *level_info, char **error) {
    Token token = get_token(tokenizer);

    if (!(token.type == KEYWORD && string_equals(token.string, "materials"))) {
        return level_parse_error(error, "Expected materials keyword.");
    }

    token = get_token(tokenizer);
    
    if (token.type != OPEN_BRACKET) {
        return level_parse_error(error, "Expected open bracket for materials block.");
    }

    bool32 parse_material_result;
    do {
        Material_Info material_info;
        parse_material_result = parse_material(temp_allocator, tokenizer,
                                               &material_info, error);

        if (parse_material_result) {
            level_info_add_material(level_info, material_info);
            
            if (token.type == CLOSE_BRACKET) {
                break;
            }
        } else {
            // parse_material() sets error message
            return false;
        }
    } while (parse_material_result);

    return true;
}

bool32 parse_keyword(Allocator *temp_allocator, Tokenizer *tokenizer, char *token_string, char **error) {
    Token token = get_token(tokenizer);
    if (!(token.type == KEYWORD && string_equals(token.string, token_string))) {
        char *error_string = string_format(temp_allocator, "Expected %s keyword.", token_string);
        return level_parse_error(error, error_string);
    }

    return true;
}

bool32 parse_vec3_property(Allocator *temp_allocator, Tokenizer *tokenizer,
                           char *property_name, Vec3 *result,
                           char **error) {
    if (!parse_keyword(temp_allocator, tokenizer, property_name, error)) {
        return false;
    }
    
    if (!level_parse_vec3(tokenizer, result, error)) {
        return false;
    }

    return true;
}

#if 0
bool32 parse_normal_entity_properties(Allocator *temp_allocator, Tokenizer *tokenizer,
                                      Normal_Entity *normal_entity, char **error) {
    // TODO: this is weird, redo this stuff below
    do {
        token = get_token(tokenizer);
        if (token.type == KEYWORD && string_equals(token.string, "type")) {
            return level_parse_error(error, "Entity type already set.");
        } else if (string_equals(token.string, "")) {
            token = get_token(tokenizer);
            
            if (token.type == INTEGER) {
                uint32 result;
                bool32 parse_result = ascii_to_uint32(token.string, &result);
                if (parse_result) {
                    material_info.use_albedo_texture = result;
                } else {
                    return level_parse_error(error, "Expected unsigned integer for use_albedo_texture.");
                }
            } else {
                return level_parse_error(error, "Expected unsigned integer for use_albedo_texture.");
            }
        } else if (string_equals(token.string, "albedo_texture")) {
}
        #endif

bool32 parse_entity(Allocator *temp_allocator, Tokenizer *tokenizer,
                    Entity_Info *entity_info_out, char **error) {
    Token token = get_token(tokenizer);

    if (token.type == CLOSE_BRACKET) {
        return true;
    }
    
    if (!(token.type == KEYWORD && string_equals(token.string, "entity"))) {
        return level_parse_error(error, "Expected entity keyword or close bracket.");
    }

    token = get_token(tokenizer);
    
    if (token.type != OPEN_BRACKET) {
        return level_parse_error(error, "Expected open bracket for entity block.");
    }

    Entity_Info entity_info;

    entity_info.transform = make_transform();

    while (true) {
        token = get_token(tokenizer);

        if (token.type == CLOSE_BRACKET) {
            return true;
        }

        if (token.type != KEYWORD) {
            *error = "Expected either keyword for entity property or close bracket for entity block."
            return false;
        }

        if (string_equals(token.string, "position")) {
            bool parse_result = level_parse_vec3(tokenizer, &entity_info.transform.position, error);
            if (!parse_result) {
                return false;
            }
        } else if (string_equals(token.string, "rotation")) {
            bool parse_result = level_parse_quaternion(tokenizer, &entity_info.transform.quaternion, error);
            if (!parse_result) {
                return false;
            }
        } else if (string_equals(token.string, "scale")) {
            bool parse_result = level_parse_vec3(tokenizer, &entity_info.transform.position, error);
            if (!parse_result) {
                return false;
            }
        } else if (string_equals(token.string, "mesh")) {
            token = get_token(tokenizer);
            
            if (token.type != STRING) {
                return level_parse_error(error, "Expected entity mesh name to be a string.");
            }

            if (token.string.length == 0) {
                return level_parse_error(error, "Entity mesh name cannot be empty.");
            }

            entity_info.flags |= ENTITY_MESH;
            entity_info.mesh_name = token.string;
        } else if (string_equals(token.string, "material")) {
            token = get_token(tokenizer);
            
            if (token.type != STRING) {
                return level_parse_error(error, "Expected entity material name to be a string.");
            }

            if (token.string.length == 0) {
                return level_parse_error(error, "Entity material name cannot be empty.");
            }

            entity_info.flags |= ENTITY_MATERIAL;
            entity_info.material_name = token.string;
        } else if (string_equals(token.string, "light_type")) {
            token = get_token(tokenizer);

            if (token.type != KEYWORD) {
                // we just have point lights currently
                return level_parse_error(error, "Expected light_type property to have a keyword value of \"point\"".);
            }

            entity_info.flags |= ENTITY_LIGHT;

            if (string_equals(token.string, "point")) {
                entity_info.light_type = LIGHT_POINT;
            } else {
                return level_parse_error(error, "Invalid light_type property value. Expected \"point\".");
            }
        } else if (string_equals(token.string, "light_color")) {
            // TODO: do this, falloff_start, and falloff_end
            // TODO: maybe figure out a way to verify that all the correct light fields are set for the light type?
        }
    }

    // TODO: remove this stuff below
    do {
        if (token.type == CLOSE_BRACKET) {
            return true;
        }

        if (token.t
        
        token = get_token(tokenizer);
    } while (token.type == KEYWORD);
    
    if (entity_info.type == ENTITY_NORMAL) {
        
    }
    
    // TODO: set defaults based on entity type
    entity_info.transform = make_transform();

    // TODO: replace these with entity parameters
    //       - make sure to check for entity type
    if (token.type == KEYWORD) {
        if (string_equals(token.string, "name")) {
            token = get_token(tokenizer);

            if (token.type == STRING) {
                if (!is_empty(token.string)) {
                    material_info.name = token.string;
                } else {
                    return level_parse_error(error, "Material name cannot be empty.");
                }
            } else {
                return level_parse_error(error, "Expected string for material name.");
            }
        } else {
            return level_parse_error(error, "Expected name keyword.");
        }
    } else {
        return level_parse_error(error, "Expected name keyword.");
    }

    // we accept duplicates of parameters and just set the material's parameter to the last one read
    // (except for the name parameter)
    do {
        token = get_token(tokenizer);
        if (string_equals(token.string, "name")) {
            return level_parse_error(error, "Material name already set.");
        } else if (string_equals(token.string, "use_albedo_texture")) {
            token = get_token(tokenizer);
            
            if (token.type == INTEGER) {
                uint32 result;
                bool32 parse_result = ascii_to_uint32(token.string, &result);
                if (parse_result) {
                    material_info.use_albedo_texture = result;
                } else {
                    return level_parse_error(error, "Expected unsigned integer for use_albedo_texture.");
                }
            } else {
                return level_parse_error(error, "Expected unsigned integer for use_albedo_texture.");
            }
        } else if (string_equals(token.string, "albedo_texture")) {
            token = get_token(tokenizer);
            
            String result;

            if (token.type == STRING) {
                if (!is_empty(token.string)) {
                    material_info.albedo_texture_name = token.string;
                } else {
                    return level_parse_error(error, "Albedo texture name cannot be empty.");
                }
            } else {
                return level_parse_error(error, "Expected string for albedo texture name.");
            }
        } else if (string_equals(token.string, "albedo_color")) {
            Vec3 result;

            bool32 parse_result = level_parse_vec3(tokenizer, &result, error);
            if (parse_result) {
                material_info.albedo_color = result;
            } else {
                // error message gets set by level_parse_vec3()
                return false;
            }
        } else if (string_equals(token.string, "use_metalness_texture")) {
            token = get_token(tokenizer);
            
            if (token.type == INTEGER) {
                uint32 result;
                bool32 parse_result = ascii_to_uint32(token.string, &result);
                if (parse_result) {
                    material_info.use_metalness_texture = result;
                } else {
                    return level_parse_error(error, "Expected unsigned integer for use_metalness_texture.");
                }
            } else {
                return level_parse_error(error, "Expected unsigned integer for use_metalness_texture.");
            }
        } else if (string_equals(token.string, "metalness_texture")) {
            token = get_token(tokenizer);
            
            String result;

            if (token.type == STRING) {
                if (!is_empty(token.string)) {
                    material_info.metalness_texture_name = token.string;
                } else {
                    return level_parse_error(error, "Metalness texture name cannot be empty.");
                }
            } else {
                return level_parse_error(error, "Expected string for metalness texture name.");
            }
        } else if (string_equals(token.string, "metalness")) {
            token = get_token(tokenizer);

            if (token.type == REAL || token.type == INTEGER) {
                real32 result;
                bool32 parse_result = string_to_real32(token.string, &result);

                if (parse_result) {
                    material_info.metalness = result;
                } else {
                    return level_parse_error("Invalid number for metalness value.");
                }
            } else {
                return level_parse_error("Expected number for metalness value.");
            }
        } else if (string_equals(token.string, "use_roughness_texture")) {
            token = get_token(tokenizer);
            
            if (token.type == INTEGER) {
                uint32 result;
                bool32 parse_result = ascii_to_uint32(token.string, &result);
                if (parse_result) {
                    material_info.use_roughness_texture = result;
                } else {
                    return level_parse_error(error, "Expected unsigned integer for use_roughness_texture.");
                }
            } else {
                return level_parse_error(error, "Expected unsigned integer for use_roughness_texture.");
            }
        } else if (string_equals(token.string, "roughness_texture")) {
            token = get_token(tokenizer);
            
            String result;

            if (token.type == STRING) {
                if (!is_empty(token.string)) {
                    material_info.roughness_texture_name = token.string;
                } else {
                    return level_parse_error(error, "Roughness texture name cannot be empty.");
                }
            } else {
                return level_parse_error(error, "Expected string for roughness texture name.");
            }
        } else if (string_equals(token.string, "roughness")) {
            token = get_token(tokenizer);

            if (token.type == REAL || token.type == INTEGER) {
                real32 result;
                bool32 parse_result = string_to_real32(token.string, &result);

                if (parse_result) {
                    material_info.roughness = result;
                } else {
                    return level_parse_error("Invalid number for roughness value.");
                }
            } else {
                return level_parse_error("Expected number for roughness value.");
            }
        } else {
            *error = "Unrecognized material property "
                }
    } while (token.type == KEYWORD);

    *material_info_out = material_info;
    return true;
}

bool32 parse_entities_block(Allocator *temp_allocator, Tokenizer *tokenizer,
                            Level_Info *level_info, char **error) {
    Token token = get_token(tokenizer);

    if (!(token.type == KEYWORD && string_equals(token.string, "entities"))) {
        return level_parse_error(error, "Expected entities keyword.");
    }

    token = get_token(tokenizer);
    
    if (token.type != OPEN_BRACKET) {
        return level_parse_error(error, "Expected open bracket for entities block.");
    }

    bool32 parse_entity_result;
    do {
        Entity_Info entity_info;
        parse_material_result = parse_entity(temp_allocator, tokenizer,
                                             &entity_info, error);

        if (parse_entity_result) {
            if (token.type == CLOSE_BRACKET) {
                break;
            }

            level_info_add_entity(level_info, entity_info);
        } else {
            // parse_entity() sets error message
            return false;
        }
    } while (parse_entity_result);

    return true;
}

bool32 Level_Loader::parse_level(Allocator *temp_allocator, File_Data file_data, Level_Info *level_info,
                                 char **error_out) {
    Tokenizer tokenizer = make_tokenizer(file_data);

    // TODO: implement/refactor this
    
    #if 0
    Normal_Entity_Info temp_normal_entity_info = {};
    Point_Light_Entity_Info temp_point_light_entity_info = {};
    Mesh_Info temp_mesh_info = {};
    Texture_Info temp_texture_info = {};
    Material_Info temp_material_info = {};

    Entity *temp_entity = NULL;

    Vec3 vec3_buffer = {};
    Quaternion quaternion_buffer = {};
    int32 num_values_read = 0;
    bool32 *bool_to_edit = NULL;

    bool32 should_add_new_temp_entity = true;
    bool32 should_add_new_temp_material = false;

    init_level_info(temp_allocator, level_info);
#endif

    bool32 result;
    
    result = parse_level_info_block(temp_allocator, &tokenizer, level_info, error_out);
    if (!result) return false;

    result = parse_meshes_block(temp_allocator, &tokenizer, level_info, error_out);
    if (!result) return false;

    result = parse_textures_block(temp_allocator, &tokenizer, level_info, error_out);
    if (!result) return false;

    result = parse_materials_block(temp_allocator, &tokenizer, level_info, error_out);
    if (!result) return false;
    
    // TODO: implement this
    result = parse_entities_block(temp_allocator, &tokenizer, level_info, error_out);
    if (!result) return false;





    // TODO: remove all this below
    do {
        token = get_token(&tokenizer);

        if (token.type == COMMENT) continue;

        switch (state) {
            case PARSE_LEVEL_INFO: {
                bool32 result = parse_level_info_block(temp_allocator, token, &tokenizer,
                                                       level_info, error_out);
                if (!result) return false;

                state = PARSE_MESH_INFO;
            } break;
            case PARSE_MESH_INFO: {
                
            } break;
        }
        
        switch (state) {
            case WAIT_FOR_LEVEL_INFO_BLOCK_NAME: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "level_info")) {
                    state = WAIT_FOR_LEVEL_INFO_BLOCK_OPEN;
                } else {
                    assert(!"Expected level_info keyword to open block.");
                }
            } break;
            case WAIT_FOR_LEVEL_INFO_BLOCK_OPEN: {
                if (token.type == OPEN_BRACKET) {
                    state = WAIT_FOR_LEVEL_NAME_KEYWORD;
                } else {
                    assert(!"Expected open bracket.");
                }
            } break;
            case WAIT_FOR_LEVEL_NAME_KEYWORD: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "level_name")) {
                    state = WAIT_FOR_LEVEL_NAME_STRING;
                } else {
                    assert(!"Expected level_name keyword.");
                }
            } break;
            case WAIT_FOR_LEVEL_NAME_STRING: {
                if (token.type == STRING) {
                    level_info->name = token.string;
                    state = WAIT_FOR_LEVEL_INFO_BLOCK_CLOSE;
                } else {
                    assert (!"Expected level name string.");
                }
            } break;
            case WAIT_FOR_LEVEL_INFO_BLOCK_CLOSE: {
                if (token.type == CLOSE_BRACKET) {
                    state = WAIT_FOR_MESHES_BLOCK_NAME;
                } else {
                    assert(!"Expected closing bracket for level_info block.");
                }
            } break;
            case WAIT_FOR_MESHES_BLOCK_NAME: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "meshes")) {
                    state = WAIT_FOR_MESHES_BLOCK_OPEN;
                } else {
                    assert (!"Expected meshes keyword to open block.");
                }
            } break;
            case WAIT_FOR_MESHES_BLOCK_OPEN: {
                if (token.type != OPEN_BRACKET) {
                    assert(!"Expected open bracket.");
                    // TODO: don't use assert, use error strings and return
                } else {
                    while (token.type != CLOSE_BRACKET) {
                        // TODO: finish this
                        token = parse_mesh(tokenizer, level_info);
                    }
                }
            } break;
            case PARSE_MESHES: {
                while (token.type 
                if (token.type == CLOSE_BRACKET) {

                }
                parse_mesh(&tokenizer, level_info);

                #if 0
                if (token.type == KEYWORD &&
                    string_equals(token.string, "mesh")) {
                    state = WAIT_FOR_MESH_NAME_STRING;
                } else if (token.type == CLOSE_BRACKET) {
                    state = WAIT_FOR_TEXTURES_BLOCK_NAME;
                } else {
                    assert(!"Expected mesh keyword or closing bracket.");
                }
                #endif
            } break;
            case WAIT_FOR_MESH_NAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= MESH_NAME_MAX_SIZE);
                    assert(!mesh_name_exists(level_info, token.string));

                    temp_mesh_info = {};
                    temp_mesh_info.name = token.string;
                    state = WAIT_FOR_MESH_FILENAME_STRING;
                } else {
                    assert(!"Expected mesh name string.");
                }
            } break;
            case WAIT_FOR_MESH_FILENAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= PLATFORM_MAX_PATH);

                    temp_mesh_info.filename = token.string;
                    add(&level_info->meshes, temp_mesh_info);

                    state = WAIT_FOR_MESH_KEYWORD_OR_MESHES_BLOCK_CLOSE;
                }
            } break;
            case WAIT_FOR_TEXTURES_BLOCK_NAME: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "textures")) {
                    state = WAIT_FOR_TEXTURES_BLOCK_OPEN;
                } else {
                    assert (!"Expected textures keyword to open block.");
                }
            } break;
            case WAIT_FOR_TEXTURES_BLOCK_OPEN: {
                if (token.type == OPEN_BRACKET) {
                    state = WAIT_FOR_TEXTURE_KEYWORD_OR_TEXTURES_BLOCK_CLOSE;
                } else {
                    assert(!"Expected open bracket.");
                }
            } break;
            case WAIT_FOR_TEXTURE_KEYWORD_OR_TEXTURES_BLOCK_CLOSE: {
                if (token.type == KEYWORD && string_equals(token.string, "texture")) {
                    temp_texture_info = {};
                    state = WAIT_FOR_TEXTURE_NAME_STRING;
                } else if (token.type == CLOSE_BRACKET) {
                    state = WAIT_FOR_MATERIALS_BLOCK_NAME;
                } else {
                    assert(!"Expected texture keyword or closing bracket.");
                }
            } break;
            case WAIT_FOR_TEXTURE_NAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= TEXTURE_NAME_MAX_SIZE);
                    assert(!texture_name_exists(level_info, token.string));

                    temp_texture_info.name = token.string;
                    state = WAIT_FOR_TEXTURE_FILENAME_STRING;
                } else {
                    assert(!"Expected texture name string.");
                }
            } break;
            case WAIT_FOR_TEXTURE_FILENAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= PLATFORM_MAX_PATH);

                    temp_texture_info.filename = token.string;
                    add(&level_info->textures, temp_texture_info);

                    state = WAIT_FOR_TEXTURE_KEYWORD_OR_TEXTURES_BLOCK_CLOSE;
                }
            } break;

            // MATERIALS
            case WAIT_FOR_MATERIALS_BLOCK_NAME: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "materials")) {
                    state = WAIT_FOR_MATERIALS_BLOCK_OPEN;
                } else {
                    assert(!"Expected materials keyword to open block.");
                }
            } break;
            case WAIT_FOR_MATERIALS_BLOCK_OPEN: {
                if (token.type == OPEN_BRACKET) {
                    state = WAIT_FOR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE;
                } else {
                    assert(!"Expected open bracket.");
                }
            } break;
            case WAIT_FOR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE: {
                if (token.type == KEYWORD && string_equals(token.string, "material")) {
                    temp_material_info = {};
                    temp_material_info.material = make_material();
                    state = WAIT_FOR_MATERIAL_NAME_STRING;
                } else if (token.type == CLOSE_BRACKET) {
                    state = WAIT_FOR_ENTITIES_BLOCK_NAME;
                } else {
                    assert(!"Expected material keyword or closing bracket.");
                }
            } break;
            case WAIT_FOR_MATERIAL_NAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= MATERIAL_NAME_MAX_SIZE);
                    assert(!material_name_exists(level_info, token.string));

                    temp_material_info.name = token.string;
                    should_add_new_temp_material = true;

                    state = WAIT_FOR_MATERIAL_PROPERTY_NAME_OR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE;
                } else {
                    assert(!"Expected material name string.");
                }
            } break;
            case WAIT_FOR_MATERIAL_PROPERTY_NAME_OR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE: {
                if (token.type == KEYWORD) {
                    if (string_equals(token.string, "material")) {
                        if (should_add_new_temp_material) {
                            add(&level_info->materials, temp_material_info);
                        }
                        state = WAIT_FOR_MATERIAL_NAME_STRING;
                    } else if (string_equals(token.string, "texture")) {
                        state = WAIT_FOR_MATERIAL_TEXTURE_NAME_STRING;
                    } else if (string_equals(token.string, "gloss")) {
                        state = WAIT_FOR_MATERIAL_GLOSS_NUMBER;
                    } else if (string_equals(token.string, "color_override")) {
                        state = WAIT_FOR_MATERIAL_COLOR_OVERRIDE_VEC3;
                        num_values_read = 0;
                    } else if (string_equals(token.string, "use_color_override")) {
                        state = WAIT_FOR_MATERIAL_USE_COLOR_OVERRIDE_INTEGER;
                    } else {
                        assert(!"Unrecognized material property.");
                    }
                } else if (token.type == CLOSE_BRACKET) {
                    if (should_add_new_temp_material) {
                        add(&level_info->materials, temp_material_info);
                    }
                    state = WAIT_FOR_ENTITIES_BLOCK_NAME;
                } else {
                    assert(!"Expected a material property name keyword.");
                }
            } break;
            case WAIT_FOR_MATERIAL_TEXTURE_NAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= TEXTURE_NAME_MAX_SIZE);

                    temp_material_info.flags |= HAS_TEXTURE;
                    temp_material_info.texture_name = token.string;

                    state = WAIT_FOR_MATERIAL_PROPERTY_NAME_OR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE;
                } else {
                    assert(!"Expected a string for material texture name.");
                }
            } break;
            case WAIT_FOR_MATERIAL_GLOSS_NUMBER: {
                if (token.type == REAL || token.type == INTEGER) {
                    temp_material_info.material.gloss = string_to_real32(token.string);
                    state = WAIT_FOR_MATERIAL_PROPERTY_NAME_OR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE;
                } else {
                    assert(!"Expected a number for material property gloss.");
                }
            } break;
            case WAIT_FOR_MATERIAL_COLOR_OVERRIDE_VEC3: {
                if (token.type == REAL || token.type == INTEGER) {
                    vec3_buffer.values[num_values_read] = string_to_real32(token.string);
                    num_values_read++;
                    if (num_values_read == 3) {
                        temp_material_info.material.color_override = make_vec4(vec3_buffer, 1.0f);
                        state = WAIT_FOR_MATERIAL_PROPERTY_NAME_OR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE;
                    }
                } else {
                    assert(!"Expected 3 numbers for material property color_override.");
                }
            } break;
            case WAIT_FOR_MATERIAL_USE_COLOR_OVERRIDE_INTEGER: {
                if (token.type == INTEGER) {
                    temp_material_info.material.use_color_override = (int32) string_to_uint32(token.string);

                    state = WAIT_FOR_MATERIAL_PROPERTY_NAME_OR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE;
                } else {
                    assert(!"Expected an integer for material property use_color_override.");
                }
            } break;
            // ENTITIES
            case WAIT_FOR_ENTITIES_BLOCK_NAME: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "entities")) {
                    state = WAIT_FOR_ENTITIES_BLOCK_OPEN;
                } else {
                    assert(!"Expected entities keyword for entities block.");
                }
            } break;
            case WAIT_FOR_ENTITIES_BLOCK_OPEN: {
                if (token.type == OPEN_BRACKET) {
                    state = WAIT_FOR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                } else {
                    assert(!"Expected open bracket for entities block.");
                }
            } break;
            case WAIT_FOR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "type")) {
                    state = WAIT_FOR_ENTITY_TYPE_VALUE;
                } else if (token.type == CLOSE_BRACKET) {
                    state = FINISHED;
                } else {
                    assert(!"Expected type keyword or closing bracket for entities block.");
                }
            } break;
            case WAIT_FOR_ENTITY_TYPE_VALUE: {
                if (token.type == KEYWORD) {
                    if (string_equals(token.string, "normal")) {
                        temp_normal_entity_info = {};
                        temp_normal_entity_info.entity = make_normal_entity();
                        temp_entity = (Entity *) &temp_normal_entity_info.entity;

                        state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                        should_add_new_temp_entity = true;
                    } else if (string_equals(token.string, "point_light")) {
                        temp_point_light_entity_info = {};
                        temp_point_light_entity_info.entity = make_point_light_entity();

                        temp_entity = (Entity *) &temp_point_light_entity_info.entity;

                        state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                        should_add_new_temp_entity = true;
                    } else {
                        assert(!"Unrecognized entity type.");
                    }
                } else {
                    assert(!"Expected entity type value keyword.");
                }
            } break;

            case WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE: {
                if ((token.type == KEYWORD && string_equals(token.string, "type")) ||
                    (token.type == CLOSE_BRACKET)) {
                    if (should_add_new_temp_entity) {
                        if (temp_entity->type == ENTITY_NORMAL) {
                            add(&level_info->normal_entities, temp_normal_entity_info);
                        } else if (temp_entity->type == ENTITY_POINT_LIGHT) {
                            add(&level_info->point_light_entities, temp_point_light_entity_info);
                        } else {
                            assert(!"Unhandled entity type.");
                        }
                    }

                    if (token.type == CLOSE_BRACKET) {
                        state = FINISHED;  
                    } else {
                        state = WAIT_FOR_ENTITY_TYPE_VALUE;
                    }
                } else if (token.type == KEYWORD) {
                    if (string_equals(token.string, "position")) {
                        num_values_read = 0;
                        state = WAIT_FOR_ENTITY_POSITION_VEC3;
                    } else if (string_equals(token.string, "rotation")) {
                        num_values_read = 0;
                        state = WAIT_FOR_ENTITY_ROTATION_QUATERNION;
                    } else if (string_equals(token.string, "scale")) {
                        num_values_read = 0;
                        state = WAIT_FOR_ENTITY_SCALE_VEC3;
                    } else if (temp_entity->type == ENTITY_NORMAL) {
                        if (string_equals(token.string, "mesh")) {
                            state = WAIT_FOR_NORMAL_ENTITY_MESH_NAME_STRING;
                        } else if (string_equals(token.string, "mesh_primitive")) {
                            state = WAIT_FOR_NORMAL_ENTITY_MESH_NAME_STRING;
                        } else if (string_equals(token.string, "material")) {
                            state = WAIT_FOR_NORMAL_ENTITY_MATERIAL_NAME_STRING;                        
                        } else if (string_equals(token.string, "is_walkable")) {
                            bool_to_edit = &temp_normal_entity_info.entity.is_walkable;
                            state = WAIT_FOR_ENTITY_BOOL;
                        } else {
                            assert(!"Unexpected entity keyword.");
                        }
                    } else if (temp_entity->type == ENTITY_POINT_LIGHT) {
                        if (string_equals(token.string, "light_color")) {
                            num_values_read = 0;
                            state = WAIT_FOR_POINT_LIGHT_ENTITY_LIGHT_COLOR_VEC3;
                        } else if (string_equals(token.string, "falloff_start")) {
                            num_values_read = 0;
                            state = WAIT_FOR_POINT_LIGHT_ENTITY_FALLOFF_START_NUMBER;
                        } else if (string_equals(token.string, "falloff_end")) {
                            num_values_read = 0;
                            state = WAIT_FOR_POINT_LIGHT_ENTITY_FALLOFF_END_NUMBER;
                        } else {
                            assert(!"Unrecognized entity property name.");
                        }
                    } else {
                        assert(!"Unrecognized entity property name.");
                    }
                } else {
                    assert(!"Expected an entity property name or a close bracket");
                }
            } break;
            case WAIT_FOR_NORMAL_ENTITY_MESH_NAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= MESH_NAME_MAX_SIZE);

                    temp_normal_entity_info.flags |= HAS_MESH;
                    temp_normal_entity_info.mesh_name = token.string;

                    state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                } else {
                    assert(!"Expected string for entity mesh name.");
                }
            } break;
            case WAIT_FOR_NORMAL_ENTITY_MATERIAL_NAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= MATERIAL_NAME_MAX_SIZE);

                    temp_normal_entity_info.flags |= HAS_MATERIAL;
                    temp_normal_entity_info.material_name = token.string;

                    state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                } else {
                    assert(!"Expected string for entity material name.");
                }                
            } break;
            case WAIT_FOR_ENTITY_POSITION_VEC3: {
                if (token.type == REAL || token.type == INTEGER) {
                    vec3_buffer.values[num_values_read] = string_to_real32(token.string);
                    num_values_read++;
                    if (num_values_read == 3) {
                        temp_entity->transform.position = vec3_buffer;
                        state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                    }
                } else {
                    assert(!"Expected 3 numbers for entity property position.");
                }
            } break;
            case WAIT_FOR_ENTITY_ROTATION_QUATERNION: {
                if (token.type == REAL || token.type == INTEGER) {
                    if (num_values_read == 0) {
                        quaternion_buffer.w = string_to_real32(token.string);
                    } else if (num_values_read >= 1 && num_values_read <= 3) {
                        int32 index = num_values_read - 1;
                        quaternion_buffer.v[index] = string_to_real32(token.string);
                    }

                    num_values_read++;
                    if (num_values_read == 4) {
                        temp_entity->transform.rotation = quaternion_buffer;
                        state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                    }
                } else {
                    assert(!"Expected 4 numbers (w, x, y, z) for entity property rotation.");
                }
            } break;
            case WAIT_FOR_ENTITY_SCALE_VEC3: {
                if (token.type == REAL || token.type == INTEGER) {
                    vec3_buffer.values[num_values_read] = string_to_real32(token.string);
                    num_values_read++;
                    if (num_values_read == 3) {
                        temp_entity->transform.scale = vec3_buffer;
                        state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                    }
                } else {
                    assert(!"Expected 3 numbers for entity property scale.");
                }
            } break;
            case WAIT_FOR_ENTITY_BOOL: {
                if (token.type == INTEGER) {
                    assert(bool_to_edit);
                    *bool_to_edit = (int32) string_to_uint32(token.string);
                    state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                } else {
                    // TODO: we could add a variable that saves the name of the property for better errors
                    assert(!"Expected an integer for entity property");
                }
            } break;
            case WAIT_FOR_POINT_LIGHT_ENTITY_LIGHT_COLOR_VEC3: {
                assert(temp_entity->type == ENTITY_POINT_LIGHT);

                if (token.type == REAL || token.type == INTEGER) {
                    vec3_buffer.values[num_values_read] = string_to_real32(token.string);
                    num_values_read++;
                    if (num_values_read == 3) {
                        temp_point_light_entity_info.entity.light_color = vec3_buffer;
                        state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                    }
                } else {
                    assert(!"Expected 3 numbers for point light entity property light_color.");
                }
            } break;
            case WAIT_FOR_POINT_LIGHT_ENTITY_FALLOFF_START_NUMBER: {
                assert(temp_entity->type == ENTITY_POINT_LIGHT);

                if (token.type == REAL || token.type == INTEGER) {
                    temp_point_light_entity_info.entity.falloff_start = string_to_real32(token.string);
                    state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                } else {
                    assert(!"Expected a number for point light entity property falloff_start.");
                }
            } break;
            case WAIT_FOR_POINT_LIGHT_ENTITY_FALLOFF_END_NUMBER: {
                assert(temp_entity->type == ENTITY_POINT_LIGHT);

                if (token.type == REAL || token.type == INTEGER) {
                    temp_point_light_entity_info.entity.falloff_end = string_to_real32(token.string);
                    state = WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE;
                } else {
                    assert(!"Expected a number for point light entity property falloff_end.");
                }
            } break;
            case FINISHED: {
                if (token.type != END) assert(!"Unexpected extra tokens.");
            }
        }

    } while (token.type != END);

    return true;
}

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

void append_default_entity_info(Editor_Level *level, String_Buffer *buffer, Entity *entity) {
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

void export_level(Asset_Manager *asset_manager, Editor_Level *level, char *filename) {
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
