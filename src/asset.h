#ifndef ASSET_H
#define ASSET_H

#include "mesh.h"
#include "font.h"

#define MAX_MESHES 64
#define MAX_ASSET_STRING_SIZE 64

struct Asset_Manager {
    Allocator *allocator_pointer;
    
    Hash_Table<int32, Mesh> mesh_table;
    Hash_Table<String, File_Data> font_file_table;
    Hash_Table<String, Font> font_table;
};

struct Texture {
    String name;
    String filename;
    bool32 is_loaded;
    bool32 should_unload;
};

struct Material {
    String name;
    int32 texture_id;
    real32 gloss;
    Vec4 color_override;
    bool32 use_color_override;
};

Material make_material() {
    Material result = {};
    result.texture_id = -1;
    return result;
}

#endif
