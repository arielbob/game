#ifndef MESH_H
#define MESH_H

#include "math.h"

#define MESH_NAME_MAX_SIZE 256

// TODO: we need more types of Mesh objects. since not all meshes require all the data. for example, a nav mesh
//       doesn't need UVs. and something like a rock mesh won't need joint data. we will also need to modify
//       our mesh loading filetype and loading to acommodate these different types.
struct Mesh {
    uint32 name_size;
    char *name;

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
};

#define MAX_TOKEN_TEXT_SIZE 1024

#endif