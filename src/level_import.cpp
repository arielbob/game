#include "level.h"

#if 0
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
#endif

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

void level_info_add_entity(Level_Info *level_info, Entity_Info entity) {
    assert(level_info->num_entities < MAX_LEVEL_INFO_ARRAY_SIZE);
    level_info->entities[level_info->num_entities++] = entity;
}

#if 0
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
#endif

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
    } while (token.type == COMMENT);

    return token;
}

Level_Loader::Token Level_Loader::peek_token(Tokenizer *tokenizer) {
    Tokenizer original = *tokenizer;

    Token peeked = get_token(tokenizer);
    
    *tokenizer = original;
    
    return peeked;
}

inline bool32 level_parse_error(char **error_out, char *error_string) {
    *error_out = error_string;
    return false;
}

bool32 Level_Loader::parse_real(Tokenizer *tokenizer, real32 *result, char **error) {
    Token token = get_token(tokenizer);
    real32 num;
    
    if (token.type == REAL || token.type == INTEGER) {
        bool32 parse_result = string_to_real32(token.string, &num);

        if (!parse_result) {
            return level_parse_error(error, "Invalid number value.");
        } else {
            *result = num;
            return true;
        }
    } else {
        return level_parse_error(error, "Expected number.");
    }
}

bool32 Level_Loader::parse_vec3(Tokenizer *tokenizer, Vec3 *result, char **error) {
    // don't set vec using result pointer since we may fail in the middle of parsing
    // and don't want to set result to a partially done vector
    Vec3 v;
    
    for (int i = 0; i < 3; i++) {
        if (!parse_real(tokenizer, &v[i], error)) {
            // we do lose some specificity in the error message here, but it's better in that
            // we know that it's an error in setting a vec3 value instead of just saying
            // "Expected number."
            return level_parse_error(error, "Invalid number in Vec3.");
        }
    }

    *result = v;
    return true;
}

bool32 Level_Loader::parse_quaternion(Tokenizer *tokenizer, Quaternion *result, char **error) {
    Quaternion quat;
    
    if (!parse_real(tokenizer, &quat.w, error)) {
        return false;
    }
    
    for (int i = 0; i < 3; i++) {
        if (!parse_real(tokenizer, &(quat.v[i]), error)) {
            return false;
        }
    }

    *result = quat;
    return true;
}

bool32 Level_Loader::parse_level_info_block(Allocator *temp_allocator, Tokenizer *tokenizer,
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

    level_info->name = token.string;
    token = get_token(tokenizer);

    if (token.type != CLOSE_BRACKET) {
        return level_parse_error(error, "Expected close bracket for level_info_block");
    }

    return true;
}

bool32 Level_Loader::parse_meshes_block(Allocator *temp_allocator, Tokenizer *tokenizer,
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

    while (token.type == KEYWORD && string_equals(token.string, "mesh")) {
        token = get_token(tokenizer);
        
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

bool32 Level_Loader::parse_textures_block(Allocator *temp_allocator, Tokenizer *tokenizer,
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

    while (token.type == KEYWORD && string_equals(token.string, "texture")) {
        token = get_token(tokenizer);
        
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
bool32 Level_Loader::parse_material(Allocator *temp_allocator, Tokenizer *tokenizer,
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

    Material_Info material_info = {};

    Texture *default_texture = get_texture(ENGINE_DEBUG_TEXTURE_ID);
    
    // set defaults
    material_info.flags = 0;
    material_info.albedo_texture_name = default_texture->name;
    material_info.metalness_texture_name = default_texture->name;
    material_info.roughness_texture_name = default_texture->name;
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

    // we accept duplicates of parameters and just set the material's parameter to the last one read
    // (except for the name parameter)
    while (true) {
        token = get_token(tokenizer);

        if (token.type == CLOSE_BRACKET) {
            break;
        }

        if (token.type != KEYWORD) {
            return level_parse_error(error, "Expected either keyword for material property or close bracket for material block.");
        }

        if (string_equals(token.string, "name")) {
            return level_parse_error(error, "Material name already set.");
        } else if (string_equals(token.string, "use_albedo_texture")) {
            token = get_token(tokenizer);
            
            if (token.type == INTEGER) {
                uint32 result;
                bool32 parse_result = string_to_uint32(token.string, &result);
                if (parse_result) {
                    if (result) {
                        material_info.flags |= MATERIAL_USE_ALBEDO_TEXTURE;
                    }
                } else {
                    return level_parse_error(error, "Expected unsigned integer for use_albedo_texture.");
                }
            } else {
                return level_parse_error(error, "Expected unsigned integer for use_albedo_texture.");
            }
        } else if (string_equals(token.string, "albedo_texture")) {
            token = get_token(tokenizer);
            
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

            bool32 parse_result = parse_vec3(tokenizer, &result, error);
            if (parse_result) {
                material_info.albedo_color = result;
            } else {
                // error message gets set by parse_vec3()
                return false;
            }
        } else if (string_equals(token.string, "use_metalness_texture")) {
            token = get_token(tokenizer);
            
            if (token.type == INTEGER) {
                uint32 result;
                bool32 parse_result = string_to_uint32(token.string, &result);
                if (parse_result) {
                    if (result) {
                        material_info.flags |= MATERIAL_USE_METALNESS_TEXTURE;
                    }
                } else {
                    return level_parse_error(error, "Expected unsigned integer for use_metalness_texture.");
                }
            } else {
                return level_parse_error(error, "Expected unsigned integer for use_metalness_texture.");
            }
        } else if (string_equals(token.string, "metalness_texture")) {
            token = get_token(tokenizer);
            
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
            if (!parse_real(tokenizer, &material_info.metalness, error)) {
                return false;
            }
        } else if (string_equals(token.string, "use_roughness_texture")) {
            token = get_token(tokenizer);
            
            if (token.type == INTEGER) {
                uint32 result;
                bool32 parse_result = string_to_uint32(token.string, &result);
                if (parse_result) {
                    if (result) {
                        material_info.flags |= MATERIAL_USE_ROUGHNESS_TEXTURE;
                    }
                } else {
                    return level_parse_error(error, "Expected unsigned integer for use_roughness_texture.");
                }
            } else {
                return level_parse_error(error, "Expected unsigned integer for use_roughness_texture.");
            }
        } else if (string_equals(token.string, "roughness_texture")) {
            token = get_token(tokenizer);
            
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
            if (!parse_real(tokenizer, &material_info.roughness, error)) {
                return false;
            }
        } else {
            return level_parse_error(error, "Unrecognized material property.");
        }
    }
    
    *material_info_out = material_info;
    return true;
}

bool32 Level_Loader::parse_materials_block(Allocator *temp_allocator, Tokenizer *tokenizer,
                                           Level_Info *level_info, char **error) {
    Token token = get_token(tokenizer);

    if (!(token.type == KEYWORD && string_equals(token.string, "materials"))) {
        return level_parse_error(error, "Expected materials keyword.");
    }

    token = get_token(tokenizer);
    
    if (token.type != OPEN_BRACKET) {
        return level_parse_error(error, "Expected open bracket for materials block.");
    }

    while (true) {
        Token peeked = peek_token(tokenizer);
        if (peeked.type == CLOSE_BRACKET) {
            token = get_token(tokenizer);
            break;
        }

        Material_Info material_info;
        if (!parse_material(temp_allocator, tokenizer, &material_info, error)) {
            return false;
        }

        level_info_add_material(level_info, material_info);
    }
    
    return true;
}

bool32 Level_Loader::parse_keyword(Allocator *temp_allocator, Tokenizer *tokenizer,
                                   char *token_string, char **error) {
    Token token = get_token(tokenizer);
    if (!(token.type == KEYWORD && string_equals(token.string, token_string))) {
        char *error_string = string_format(temp_allocator, "Expected %s keyword.", token_string);
        return level_parse_error(error, error_string);
    }

    return true;
}

bool32 Level_Loader::parse_vec3_property(Allocator *temp_allocator, Tokenizer *tokenizer,
                                         char *property_name, Vec3 *result,
                                         char **error) {
    if (!parse_keyword(temp_allocator, tokenizer, property_name, error)) {
        return false;
    }
    
    if (!parse_vec3(tokenizer, result, error)) {
        return false;
    }

    return true;
}

bool32 Level_Loader::parse_entity(Allocator *temp_allocator, Tokenizer *tokenizer,
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

    Entity_Info entity_info = {};

    entity_info.transform = make_transform();

    while (true) {
        token = get_token(tokenizer);

        if (token.type == CLOSE_BRACKET) {
            break;
        }

        if (token.type != KEYWORD) {
            return level_parse_error(error, "Expected either keyword for entity property or close bracket for entity block.");
        }

        if (string_equals(token.string, "position")) {
            if (!parse_vec3(tokenizer, &entity_info.transform.position, error)) {
                return false;
            }
        } else if (string_equals(token.string, "rotation")) {
            if (!parse_quaternion(tokenizer, &entity_info.transform.rotation, error)) {
                return false;
            }
        } else if (string_equals(token.string, "scale")) {
            if (!parse_vec3(tokenizer, &entity_info.transform.scale, error)) {
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
                return level_parse_error(error, "Expected light_type property to have a keyword value of \"point\"");
            }

            entity_info.flags |= ENTITY_LIGHT;

            if (string_equals(token.string, "point")) {
                entity_info.light_type = LIGHT_POINT;
            } else if (string_equals(token.string, "sun")) {
                entity_info.light_type = LIGHT_SUN;
            } else {
                return level_parse_error(error, "Invalid light_type property value. Expected \"point\".");
            }
        } else if (string_equals(token.string, "light_color")) {
            // it's actually fine if we set these even if the specific light type doesn't have that
            // specific parameter. it'll get set, but it'll only ever actually be used if you change
            // the light type of the entity in the editor to a type that uses the parameter, which
            // is fine.
            if (!parse_vec3(tokenizer, &entity_info.light_color, error)) {
                return false;
            }
        } else if (string_equals(token.string, "falloff_start")) {
            if (!parse_real(tokenizer, &entity_info.falloff_start, error)) {
                return false;
            }
        } else if (string_equals(token.string, "falloff_end")) {
            if (!parse_real(tokenizer, &entity_info.falloff_end, error)) {
                return false;
            }
        } else if (string_equals(token.string, "sun_color")) {
            if (!parse_vec3(tokenizer, &entity_info.sun_color, error)) {
                return false;
            }
        } else if (string_equals(token.string, "sun_direction")) {
            if (!parse_vec3(tokenizer, &entity_info.sun_direction, error)) {
                return false;
            }
        } else if (string_equals(token.string, "has_collider")) {
            token = get_token(tokenizer);
            
            if (token.type == INTEGER) {
                uint32 result;
                bool32 parse_result = string_to_uint32(token.string, &result);
                if (parse_result) {
                    if (result) {
                        entity_info.flags |= ENTITY_COLLIDER;
                    }
                } else {
                    return level_parse_error(error, "Expected unsigned integer for has_collider.");
                }
            } else {
                return level_parse_error(error, "Expected unsigned integer for has_collider.");
            }
        } else {
            return level_parse_error(error, "Unrecognized entity property.");
        }
    }

    *entity_info_out = entity_info;
    return true;
}

bool32 Level_Loader::parse_entities_block(Allocator *temp_allocator, Tokenizer *tokenizer,
                                          Level_Info *level_info, char **error) {
    Token token = get_token(tokenizer);

    if (!(token.type == KEYWORD && string_equals(token.string, "entities"))) {
        return level_parse_error(error, "Expected entities keyword.");
    }

    token = get_token(tokenizer);
    
    if (token.type != OPEN_BRACKET) {
        return level_parse_error(error, "Expected open bracket for entities block.");
    }

    while (true) {
        Token peeked = peek_token(tokenizer);
        if (peeked.type == CLOSE_BRACKET) {
            token = get_token(tokenizer); // consume the close bracket
            break;
        }

        Entity_Info entity_info;
        if (!parse_entity(temp_allocator, tokenizer, &entity_info, error)) {
            return false;
        }

        level_info_add_entity(level_info, entity_info);
    }
    
    return true;
}

// TODO: modify load_level to use the new level_info format
bool32 Level_Loader::parse_level(Allocator *temp_allocator, File_Data file_data,
                                 Level_Info *level_info, char **error_out) {
    Tokenizer tokenizer = make_tokenizer(file_data);

    bool32 result;

    // it's fine to use the level_info pointer here since it's just in temp_region and we're just gonna clear it.
    // honestly, we can make the same assumption for the parse functions, i.e. we don't need to store a temp
    // variable then do a copy, but i mean, it's fine for now.
    result = parse_level_info_block(temp_allocator, &tokenizer, level_info, error_out);
    if (!result) return false;

    result = parse_meshes_block(temp_allocator, &tokenizer, level_info, error_out);
    if (!result) return false;

    result = parse_textures_block(temp_allocator, &tokenizer, level_info, error_out);
    if (!result) return false;

    result = parse_materials_block(temp_allocator, &tokenizer, level_info, error_out);
    if (!result) return false;
    
    result = parse_entities_block(temp_allocator, &tokenizer, level_info, error_out);
    if (!result) return false;

    Token token = get_token(&tokenizer);
    if (token.type != END) {
        return level_parse_error(error_out, "Expected end of level file.");
    }

    return true;
}
