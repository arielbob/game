#ifndef LEVEL_H
#define LEVEL_H

#include "parse.h"
#include "game.h"

// to fix circular dependency between game and level
// game.h includes level.h first, but level.h needs the Game_State struct
struct Game_State;

struct Level {
    String_Buffer name;

    Hash_Table<int32, Normal_Entity> normal_entity_table;
    Hash_Table<int32, Point_Light_Entity> point_light_entity_table;

    //int32 num_normal_entities;
    //Normal_Entity normal_entities[MAX_ENTITIES];

    //int32 num_point_lights;
    //Point_Light_Entity point_lights[MAX_POINT_LIGHTS];

    Arena_Allocator *arena;
    Pool_Allocator *string64_pool;
    Pool_Allocator *filename_pool;

    Hash_Table<int32, Mesh> mesh_table;
    Hash_Table<int32, Material> material_table;
    Hash_Table<int32, Texture> texture_table;
    
    bool32 should_clear_gpu_data;
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
        WAIT_FOR_ENTITY_MESH_NAME_STRING,
        WAIT_FOR_ENTITY_MATERIAL_NAME_STRING,
        WAIT_FOR_ENTITY_POSITION_VEC3,
        WAIT_FOR_ENTITY_ROTATION_QUATERNION,
        WAIT_FOR_ENTITY_SCALE_VEC3,
        WAIT_FOR_ENTITY_BOOL,

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
    Token get_token(Tokenizer *tokenizer, char *file_contents);
    bool32 load_temp_level(Allocator *temp_allocator, Game_State *game_state,
                           File_Data file_data, Level *temp_level);
};

bool32 read_and_load_level(Game_State *game_state,
                           Level *level, char *filename,
                           Arena_Allocator *mesh_arena,
                           Pool_Allocator *string64_pool,
                           Pool_Allocator *filename_pool);

#endif
