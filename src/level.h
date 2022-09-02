#ifndef LEVEL_H
#define LEVEL_H

#include "parse.h"

struct Editor_Level {
    Heap_Allocator heap;
    
    String name;
    String filename;

    Entity *entities;
    int32 total_entities_added_ever;
    
    //Linked_List<Entity *> entities;
};

struct Game_Level {
    Entity *entities;
    //Linked_List<Entity *> entities;
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

    enum Parser_State {
        PARSE_LEVEL_INFO,
        PARSE_MESH_INFO,





        // TODO: remove these
        WAIT_FOR_LEVEL_INFO_BLOCK_NAME,
        WAIT_FOR_LEVEL_INFO_BLOCK_OPEN,
        WAIT_FOR_LEVEL_NAME_KEYWORD,
        WAIT_FOR_LEVEL_NAME_STRING,
        WAIT_FOR_LEVEL_INFO_BLOCK_CLOSE,

        WAIT_FOR_MESHES_BLOCK_NAME,
        WAIT_FOR_MESHES_BLOCK_OPEN,
        WAIT_FOR_MESH_KEYWORD_OR_MESHES_BLOCK_CLOSE,
        WAIT_FOR_MESH_NAME_STRING,
        WAIT_FOR_MESH_FILENAME_STRING,

        WAIT_FOR_TEXTURES_BLOCK_NAME,
        WAIT_FOR_TEXTURES_BLOCK_OPEN,
        WAIT_FOR_TEXTURE_KEYWORD_OR_TEXTURES_BLOCK_CLOSE,
        WAIT_FOR_TEXTURE_NAME_STRING,
        WAIT_FOR_TEXTURE_FILENAME_STRING,

        WAIT_FOR_MATERIALS_BLOCK_NAME,
        WAIT_FOR_MATERIALS_BLOCK_OPEN,
        WAIT_FOR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE,
        WAIT_FOR_MATERIAL_NAME_STRING,
        WAIT_FOR_MATERIAL_PROPERTY_NAME_OR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE,

        WAIT_FOR_MATERIAL_TEXTURE_NAME_STRING,
        WAIT_FOR_MATERIAL_GLOSS_NUMBER,
        WAIT_FOR_MATERIAL_COLOR_OVERRIDE_VEC3,
        WAIT_FOR_MATERIAL_USE_COLOR_OVERRIDE_INTEGER,

        WAIT_FOR_ENTITIES_BLOCK_NAME,
        WAIT_FOR_ENTITIES_BLOCK_OPEN,
        WAIT_FOR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE,
        WAIT_FOR_ENTITY_TYPE_VALUE,

        WAIT_FOR_ENTITY_PROPERTY_NAME_OR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE,
        WAIT_FOR_ENTITY_POSITION_VEC3,
        WAIT_FOR_ENTITY_ROTATION_QUATERNION,
        WAIT_FOR_ENTITY_SCALE_VEC3,
        WAIT_FOR_ENTITY_BOOL,

        WAIT_FOR_NORMAL_ENTITY_MESH_NAME_STRING,
        WAIT_FOR_NORMAL_ENTITY_MATERIAL_NAME_STRING,
        
        WAIT_FOR_POINT_LIGHT_ENTITY_LIGHT_COLOR_VEC3,
        WAIT_FOR_POINT_LIGHT_ENTITY_FALLOFF_START_NUMBER,
        WAIT_FOR_POINT_LIGHT_ENTITY_FALLOFF_END_NUMBER,

        FINISHED
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
}

#endif
