#include "linked_list.h"
#include "level.h"

void init_level_info(Allocator *temp_allocator, Level_Info *level_info) {
    *level_info = {};

    make_and_init_linked_list(Normal_Entity_Info,      &level_info->normal_entities,      temp_allocator);
    make_and_init_linked_list(Point_Light_Entity_Info, &level_info->point_light_entities, temp_allocator);
    make_and_init_linked_list(Mesh_Info,               &level_info->meshes,               temp_allocator);
    make_and_init_linked_list(Texture_Info,            &level_info->textures,             temp_allocator);
    make_and_init_linked_list(Material_Info,           &level_info->materials,            temp_allocator);
}

inline Level_Loader::Token Level_Loader::make_token(Token_Type type, char *contents, int32 length) {
    Token token = {
        type,
        make_string(contents, length)
    };
    return token;
}

Level_Loader::Token Level_Loader::get_token(Tokenizer *tokenizer, char *file_contents) {
    Token token = {};

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

        token = make_token(type, &file_contents[start], length);
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
        token = make_token(COMMENT, &file_contents[start], length);
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
        token = make_token(KEYWORD, &file_contents[start], length);
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
            token = make_token(STRING, &file_contents[start], length);
        }
    } else if (*tokenizer->current == '{') {
        int32 start = tokenizer->index;
        increment_tokenizer(tokenizer);
        int32 length = tokenizer->index - start;
        token = make_token(OPEN_BRACKET, &file_contents[start], length);
    } else if (*tokenizer->current == '}') {
        int32 start = tokenizer->index;
        increment_tokenizer(tokenizer);
        int32 length = tokenizer->index - start;
        token = make_token(CLOSE_BRACKET, &file_contents[start], length);
    } else {
        assert(!"Token type not recognized.");
    }

    return token;
}

bool32 Level_Loader::parse_level_info(Allocator *temp_allocator, File_Data file_data, Level_Info *level_info) {
    Tokenizer tokenizer = make_tokenizer(file_data);

    Token token;
    Parser_State state = WAIT_FOR_LEVEL_INFO_BLOCK_NAME;

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

    do {
        token = get_token(&tokenizer, (char *) file_data.contents);

        if (token.type == COMMENT) continue;

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
                if (token.type == OPEN_BRACKET) {
                    state = WAIT_FOR_MESH_KEYWORD_OR_MESHES_BLOCK_CLOSE;
                } else {
                    assert(!"Expected open bracket.");
                }
            } break;
            case WAIT_FOR_MESH_KEYWORD_OR_MESHES_BLOCK_CLOSE: {
                if (token.type == KEYWORD &&
                    string_equals(token.string, "mesh")) {
                    state = WAIT_FOR_MESH_NAME_STRING;
                } else if (token.type == CLOSE_BRACKET) {
                    state = WAIT_FOR_TEXTURES_BLOCK_NAME;
                } else {
                    assert(!"Expected mesh keyword or closing bracket.");
                }
            } break;
            case WAIT_FOR_MESH_NAME_STRING: {
                if (token.type == STRING) {
                    assert(token.string.length <= MESH_NAME_MAX_SIZE);
                    
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
