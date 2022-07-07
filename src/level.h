#ifndef LEVEL_H
#define LEVEL_H

#include "parse.h"

#define HAS_MESH     1
#define HAS_MATERIAL 1 << 1
#define HAS_TEXTURE  1 << 2

struct Editor_Level {
    String name;
    
    Linked_List<Entity *> entities;
};

struct Game_Level {
    Linked_List<Entity *> entities;
};

void deallocate(Editor_Level *level) {
    deallocate(level->name);
    deallocate(&level->entities);
}

// we use info structs to store string identifiers to assets. since the actual entity structs use integer
// identifiers, we need to resolve them, but the integer identifiers for assets are not set until after the
// assets have been loaded. we may want to just add UUIDs for assets in the future, although those would take
// up a lot more space.

// we also need to resolve texture IDs for materials, so we store texture names in Material_Info.

// the reason why we use this intermediate structure for loading levels is so that we can choose where we store
// the data. if we're in the game, then a lot of times we can just dump things into an arena and clear it when
// the level is no longer needed. but if we're in the editor, then level data needs to be able to be modified, and
// thus we often need to use different data structures and allocators to hold the level data.
struct Normal_Entity_Info {
    Normal_Entity entity;
    uint32 flags;
    String mesh_name;
    String material_name;
};

struct Point_Light_Entity_Info {
    Point_Light_Entity entity;
};

struct Mesh_Info {
    String name;
    String filename;
};

struct Texture_Info {
    String name;
    String filename;
};

struct Level_Info {
    String name;
    
    Linked_List<Normal_Entity_Info> normal_entities;
    Linked_List<Point_Light_Entity_Info> point_light_entities;

    Linked_List<Mesh_Info> meshes;
    Linked_List<Texture_Info> textures;
    Hash_Table<String, Material> material_table;
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
    Token get_token(Tokenizer *tokenizer, char *file_contents);
    bool32 parse_level_info(Allocator *temp_allocator, File_Data file_data, Level_Info *level_info);
}

#endif
