#ifndef LEVEL_H
#define LEVEL_H

#include "parse.h"
#include "entity.h"

struct Level {
    Heap_Allocator heap;
    
    String name;
    String filename;

    Entity *entities;
    int32 total_entities_added_ever;

    bool32 is_loaded;
};

struct Mesh_Info {
    String name;
    String filename;
};

struct Texture_Info {
    String name;
    String filename;
};

struct Material_Info {
    MATERIAL_FIELDS
};

struct Entity_Info {
    // we use this to get the material, then set the entity's material_id from it
    String material_name;
    // same for meshes
    String mesh_name;

    ENTITY_FIELDS
};

#define MAX_LEVEL_INFO_ARRAY_SIZE 128
struct Level_Info {
    String name;
    String filename;
    
    int32 num_entities;
    Entity_Info entities[MAX_LEVEL_INFO_ARRAY_SIZE];

    int32 num_meshes;
    Mesh_Info meshes[MAX_LEVEL_INFO_ARRAY_SIZE];

    int32 num_textures;
    Texture_Info textures[MAX_LEVEL_INFO_ARRAY_SIZE];

    int32 num_materials;
    Material_Info materials[MAX_LEVEL_INFO_ARRAY_SIZE];
};

namespace Level_Loader {
    enum Token_Type {
        END,
        COMMENT,
        INTEGER,
        REAL,   
        KEYWORD,
        STRING, 
        OPEN_BRACKET,
        CLOSE_BRACKET
    };

    struct Token {
        Token_Type type;
        String string;
    };

    Token make_token(Token_Type type, char *contents, int32 length);
    Token get_token(Tokenizer *tokenizer);
    Token peek_token(Tokenizer *tokenizer);
    
    bool32 parse_level(Allocator *temp_allocator, File_Data file_data,
                       Level_Info *level_info, char **error_out);
    bool32 parse_level_info_block(Allocator *temp_allocator, Tokenizer *tokenizer,
                                  Level_Info *level_info, char **error);
    bool32 parse_meshes_block(Allocator *temp_allocator, Tokenizer *tokenizer,
                              Level_Info *level_info, char **error);
    bool32 parse_material(Allocator *temp_allocator, Tokenizer *tokenizer,
                          Material_Info *material_info_out, char **error);
    bool32 parse_materials_block(Allocator *temp_allocator, Tokenizer *tokenizer,
                                 Level_Info *level_info, char **error);
    bool32 parse_entity(Allocator *temp_allocator, Tokenizer *tokenizer,
                        Entity_Info *entity_info_out, char **error);
    bool32 parse_entities_block(Allocator *temp_allocator, Tokenizer *tokenizer,
                                Level_Info *level_info, char **error);
    
    bool32 parse_real(Tokenizer *tokenizer, real32 *result, char **error);
    bool32 parse_vec3(Tokenizer *tokenizer, Vec3 *result, char **error);
    bool32 parse_quaternion(Tokenizer *tokenizer, Quaternion *result, char **error);
    bool32 parse_textures_block(Allocator *temp_allocator, Tokenizer *tokenizer,
                                Level_Info *level_info, char **error);
    bool32 parse_keyword(Allocator *temp_allocator, Tokenizer *tokenizer,
                         char *token_string, char **error);
    bool32 parse_vec3_property(Allocator *temp_allocator, Tokenizer *tokenizer,
                               char *property_name, Vec3 *result,
                               char **error);
}

void load_level(Level *level, Level_Info *level_info);

#endif
