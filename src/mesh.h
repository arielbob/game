#ifndef MESH_H
#define MESH_H

#include "math.h"
#include "string.h"

#define MAX_TOKEN_TEXT_SIZE 1024

enum class Mesh_Type { NONE, LEVEL, PRIMITIVE, ENGINE };

// TODO: we need more types of Mesh objects. since not all meshes require all the data. for example, a nav mesh
//       doesn't need UVs. and something like a rock mesh won't need joint data. we will also need to modify
//       our mesh loading filetype and loading to acommodate these different types.
struct Mesh {
    Mesh_Type type;

    String_Buffer name;
    String_Buffer filename;

    Allocator *allocator;

    real32 *data;
    uint32 num_vertices;
    // size of data in bytes
    uint32 data_size;
    
    // number of components in a vertex
    uint32 n_vertex;
    uint32 n_normal;
    uint32 n_uv;
    
    // NOTE: vertex_stride = total amount of numbers for a single vertex, i.e. sum up all mesh.n_* members
    uint32 vertex_stride;

    uint32 *indices;
    uint32 num_triangles;
    uint32 indices_size;

    AABB aabb;

    bool32 is_loaded;
    bool32 should_unload;
    bool32 is_double_sided;
};

Mesh read_and_load_mesh(Allocator *allocator,
                        Mesh_Type type,
                        String_Buffer filename_buffer,
                        String_Buffer name_buffer);

#endif
