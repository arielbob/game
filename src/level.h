#ifndef LEVEL_H
#define LEVEL_H

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

void read_and_load_level(Level *level, char *filename);

#endif
