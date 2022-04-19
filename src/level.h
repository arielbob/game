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
