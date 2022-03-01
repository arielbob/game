#ifndef GAME_GL_H
#define GAME_GL_H

#include "hash_table.h"

// copy constants from here: https://www.khronos.org/registry/OpenGL/api/GL/glext.h
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_MULTISAMPLE                    0x809D

enum Shader_Type {
    VERTEX,
    FRAGMENT
};

struct GL_State {
    Hash_Table<uint32> shader_ids_table;
    Hash_Table<uint32> debug_vaos_table;
};

#endif
