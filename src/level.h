#ifndef LEVEL_H
#define LEVEL_H

#include "parse.h"

struct Level {
    String_Buffer name;

    int32 num_normal_entities;
    Normal_Entity normal_entities[MAX_ENTITIES];

    int32 num_point_lights;
    Point_Light_Entity point_lights[MAX_POINT_LIGHTS];

    Arena_Allocator mesh_arena;
    Pool_Allocator string64_pool;
    Pool_Allocator filename_pool;

    Hash_Table<int32, Mesh> mesh_table;
    Hash_Table<int32, Material> material_table;
    Hash_Table<int32, Texture> texture_table;
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

        // TODO: will have to keep track of some extra values if we're waiting for multiple values such as with
        //       vectors or colors
        WAIT_FOR_MATERIALS_BLOCK_NAME,
        WAIT_FOR_MATERIALS_BLOCK_OPEN,
        WAIT_FOR_MATERIAL_KEYWORD_OR_MATERIALS_BLOCK_CLOSE,
        WAIT_FOR_MATERIAL_NAME_STRING,
        WAIT_FOR_MATERIAL_PROPERTY_NAME,
        WAIT_FOR_MATERIAL_VALUE_STRING,
        WAIT_FOR_MATERIAL_VALUE_REAL,
        WAIT_FOR_MATERIAL_VALUE_INTEGER,

        // TODO: might need more states for this
        WAIT_FOR_ENTITIES_BLOCK_NAME,
        WAIT_FOR_ENTITIES_BLOCK_OPEN,
        WAIT_FOR_ENTITY_TYPE_KEYWORD_OR_ENTITIES_BLOCK_CLOSE,
        WAIT_FOR_ENTITY_PROPERTY_NAME,
        WAIT_FOR_ENTITY_VALUE_STRING,
        WAIT_FOR_ENTITY_VALUE_REAL
    };

    struct Token {
        Token_Type type;
        String contents;
#if 0
        char *text;
        int32 length;
#endif
    };

    Token make_token(Token_Type type, char *contents, int32 length);
    Token get_token(Tokenizer *tokenizer, char *file_contents);
    void load_level(File_Data file_data, Level *level);
};

void read_and_load_level(Level *level, char *filename);

#endif
