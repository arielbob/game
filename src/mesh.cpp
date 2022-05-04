#include "memory.h"
#include "mesh.h"
#include "platform.h"
#include "parse.h"

/*
  NOTE: FILE FORMAT
  NOTE: the actual count of the data must match the given numbers
  i.e. there should be num_vertices groups of vertex data (v, n, uv groups)
  and a total of 3 * num_triangles of indices components

  num_vertices
  num_triangles

  ;; vertices

  v -0.5 0.0 0.5
  n 0 1 0
  uv 0.0 1.0

  v 0.5 0.0 0.5
  n 0.0 1.0 0.0
  uv 1.0 1.0

  v 0.5 0.0 .5
  n 0.0 1.0 .0
  uv 1.0 1.0

  ;; indices

  0 1 2
  0 2 3
*/

/*
  4
  6

  ;; vertices

  v -0.5 0.0 0.5
  n 0 1 0
  uv 0.0 1.0

  v 0.5 0.0 0.5
  n 0.0 1.0 0.0
  uv 1.0 1.0

  v 0.5 0.0 0.5
  n 0.0 1.0 0.0
  uv 1.0 1.0

  ;; indices

  0 1 2
  0 2 3
*/

namespace Mesh_Loader {
    enum Token_Type {
        END,
        COMMENT,
        INTEGER,
        REAL,
        LABEL
    };

    enum Parser_State {
        WAITING_FOR_NUM_VERTICES,
        WAITING_FOR_NUM_TRIANGLES,
        WAITING_FOR_VERTEX_LABEL,
        WAITING_FOR_VERTEX_COMPONENTS,
        WAITING_FOR_NORMAL_LABEL,
        WAITING_FOR_NORMAL_COMPONENTS,
        WAITING_FOR_UV_LABEL,
        WAITING_FOR_UV_COMPONENTS,
        WAITING_FOR_INDICES
    };

    struct Token {
        Token_Type type;
        char *text;
        int32 length;
    };

    Token get_token(Tokenizer *tokenizer, char* file_contents);
    bool32 token_text_equals(Token token, char *str);
    Mesh load_mesh(File_Data file_data, Allocator *allocator);
}


// NOTE: token.text is not null-terminated, but str is expected to be null-terminated
inline bool32 Mesh_Loader::token_text_equals(Token token, char *str) {
    for (int32 i = 0; i < token.length; i++) {
        // NOTE: this also handles the case when token.length is larger than the length of the string, since
        //       we assume the token does not have any null characters. if we hit a null character in str,
        //       token.text[i] != '\0', so we return false.
        if (token.text[i] != str[i]) {
            return false;
        }
    }

    // NOTE: handles case where token.length is less then length of string
    //       ex: token.text = vertices
    //           str        = verticesabc
    //       this should return false since str[token.length] is not the null-terminator
    if (str[token.length] == '\0') {
        return true;
    } else {
        return false;
    }
}

Mesh_Loader::Token Mesh_Loader::get_token(Tokenizer *tokenizer, char *file_contents) {
    Token token = {};

    consume_leading_whitespace(tokenizer);

    if (is_end(tokenizer)) {
        token.type = Mesh_Loader::END;
        token.text = NULL;
        token.length = 0;
        return token;
    }
    
    char c = *tokenizer->current;

    if (is_digit(c) || c == '-' || c == '.') {
        uint32 start = tokenizer->index;
        
        bool32 has_period = false;
        bool32 is_negative = false;
        if (c == '.') {
            has_period = true;
        } else if (c == '-') {
            is_negative = true;
        }

        increment_tokenizer(tokenizer);
        
        while (!is_end(tokenizer) &&
               !is_whitespace(*tokenizer->current)) {

            if (!is_digit(*tokenizer->current) &&
                *tokenizer->current != '.') {
                assert(!"Expected digit or period");
            }
            
            if (*tokenizer->current == '.') {
                if (!has_period) {
                    has_period = true;
                } else {
                    assert(!"More than one period in number");
                }
            }

            if (*tokenizer->current == '-') {
                assert(!"Negatives can only be at the start of a number");
            }

            increment_tokenizer(tokenizer);
        }
        
        uint32 length = tokenizer->index - start;
        if (has_period) {
            token.type = REAL;
        } else {
            token.type = INTEGER;
        }

        // NOTE: we don't include the null terminator here, and just set bounds using a length member
        token.text = &file_contents[start];
        token.length = length;
    } else if (tokenizer->current[0] == ';' &&
               tokenizer->current[1] == ';') {
        increment_tokenizer(tokenizer, 2);
        uint32 start = tokenizer->index;
        
        while (!is_end(tokenizer) &&
               !is_line_end(tokenizer->current)) {
            increment_tokenizer(tokenizer);
        }

        uint32 length = tokenizer->index - start;
        token.type = COMMENT;
        token.text = &file_contents[start];
        token.length = length;
    } else if (is_letter(tokenizer->current[0])) {
        uint32 start = tokenizer->index;

        increment_tokenizer(tokenizer);
        
        while (!is_end(tokenizer) &&
               !is_whitespace(tokenizer->current[0])) {

            if (!is_letter(tokenizer->current[0])) {
                assert(!"Labels can only contain letters");
            }

            increment_tokenizer(tokenizer);
        }

        uint32 length = tokenizer->index - start;
        token.type = LABEL;
        token.text = &file_contents[start];
        token.length = length;
    } else {
        assert(!"Token type not recognized");
    }

    return token;
}

Mesh Mesh_Loader::load_mesh(File_Data file_data, Allocator *allocator) {
    Tokenizer tokenizer = {};
    tokenizer.current = (char *) file_data.contents;
    tokenizer.index = 0;
    tokenizer.size = file_data.size;

    Token token;

    Mesh mesh = {};

    mesh.n_vertex = 3;
    mesh.n_normal = 3;
    mesh.n_uv = 2;
    mesh.vertex_stride = mesh.n_vertex + mesh.n_normal + mesh.n_uv;
    
    Parser_State state = WAITING_FOR_NUM_VERTICES;

    uint32 num_components = 0;
    uint32 current_vertex = 0;
    uint32 indices_so_far = 0;

    AABB aabb = {};
    bool32 aabb_is_initialized = false;
    
    do {
        token = get_token(&tokenizer, (char *) file_data.contents);

        if (token.type == COMMENT) continue;

        // parse token
        switch (state) {
            case WAITING_FOR_NUM_VERTICES:
            {
                if (token.type == INTEGER) {
                    mesh.num_vertices = string_to_uint32(token.text, token.length);
                    mesh.data_size = mesh.num_vertices * mesh.vertex_stride * sizeof(real32);
                    mesh.data = (real32 *) allocate(allocator, mesh.data_size);

                    state = WAITING_FOR_NUM_TRIANGLES;
                } else {
                    assert(!"Expected number of vertices to be an integer");
                }
            } break;
            
            case WAITING_FOR_NUM_TRIANGLES:
            {
                if (token.type == INTEGER) {
                    mesh.num_triangles = string_to_uint32(token.text, token.length);
                    mesh.indices_size = mesh.num_triangles * 3 * sizeof(uint32);
                    mesh.indices = (uint32 *) allocate(allocator, mesh.indices_size);
                    
                    state = WAITING_FOR_VERTEX_LABEL;
                } else {
                    assert(!"Expected number of vertices to be an integer");
                }
            } break;

            case WAITING_FOR_VERTEX_LABEL:
            {
                if (token.type == LABEL &&
                    token_text_equals(token, "v")) {
                    state = WAITING_FOR_VERTEX_COMPONENTS;
                    num_components = 0;
                } else {
                    assert(!"Expected label with text \"v\"");
                }
            } break;
            
            case WAITING_FOR_VERTEX_COMPONENTS:
            {
                if (token.type == INTEGER || token.type == REAL) {
                    real32 component = string_to_real32(token.text, token.length);
                    mesh.data[current_vertex*mesh.vertex_stride + num_components] = component;

                    if (!aabb_is_initialized) {
                        aabb.p_min[num_components] = component;
                        aabb.p_max[num_components] = component;
                        if (num_components == mesh.n_vertex - 1) {
                            aabb_is_initialized = true;
                        }
                    } else {
                        aabb.p_min[num_components] = min(aabb.p_min[num_components], component);
                        aabb.p_max[num_components] = max(aabb.p_max[num_components], component);
                    }
                    
                    num_components++;
                    if (num_components == mesh.n_vertex) {
                        state = WAITING_FOR_NORMAL_LABEL;
                    }
                } else {
                    assert(!"Expected a number (integer or real) for vertex component");
                }
            } break;
            
            case WAITING_FOR_NORMAL_LABEL:
            {
                if (token.type == LABEL &&
                    token_text_equals(token, "n")) {
                    state = WAITING_FOR_NORMAL_COMPONENTS;
                    num_components = 0;
                } else {
                    assert(!"Expected label with text \"n\"");
                }
            } break;
            
            case WAITING_FOR_NORMAL_COMPONENTS:
            {
                if (token.type == INTEGER || token.type == REAL) {
                    real32 component = string_to_real32(token.text, token.length);
                    mesh.data[current_vertex*mesh.vertex_stride + mesh.n_vertex + num_components] = component;
                    
                    num_components++;
                    if (num_components == mesh.n_normal) {
                        state = WAITING_FOR_UV_LABEL;
                    }
                } else {
                    assert(!"Expected a number (integer or real) for normal component");
                }
            } break;

            case WAITING_FOR_UV_LABEL:
            {
                if (token.type == LABEL &&
                    token_text_equals(token, "uv")) {
                    state = WAITING_FOR_UV_COMPONENTS;
                    num_components = 0;
                } else {
                    assert(!"Expected label with text \"uv\"");
                }
            } break;
            
            case WAITING_FOR_UV_COMPONENTS:
            {
                if (token.type == INTEGER || token.type == REAL) {
                    real32 component = string_to_real32(token.text, token.length);
                    mesh.data[current_vertex*mesh.vertex_stride +
                              mesh.n_vertex + mesh.n_normal +
                              num_components] = component;
                    num_components++;
                    if (num_components == mesh.n_uv) {
                        // NOTE: end of a single vertex, move on to next vertex, or if we're done, onto the indices
                        current_vertex++;

                        if (current_vertex == mesh.num_vertices) {
                            // vertices are done, so aabb is set
                            mesh.aabb = aabb;
                            
                            // move onto the indices
                            state = WAITING_FOR_INDICES;
                        } else {
                            // go onto the next vertex
                            state = WAITING_FOR_VERTEX_LABEL;
                        }
                    }
                } else {
                    assert(!"Expected a number (integer or real) for UV component");
                }
            } break;
            
            case WAITING_FOR_INDICES:
            {
                if (token.type == INTEGER) {
                    uint32 vertex_index = string_to_uint32(token.text, token.length);
                    assert(vertex_index >= 0 && vertex_index < mesh.num_vertices);
                    assert(indices_so_far / 3 < mesh.num_triangles);
                    
                    mesh.indices[indices_so_far] = vertex_index;
                    indices_so_far++;
                } else if (token.type == END) {
                    assert(mesh.num_triangles * 3 == indices_so_far);
                } else {
                    assert(!"Expected integer or end of data");
                }
            } break;
        }
    } while (token.type != END);

    return mesh;
}

Vec3 get_vertex_from_index(Mesh *mesh, uint32 index) {
    assert(index < mesh->num_vertices);

    real32 x = mesh->data[mesh->vertex_stride * index];
    real32 y = mesh->data[mesh->vertex_stride * index + 1];
    real32 z = mesh->data[mesh->vertex_stride * index + 2];

    Vec3 result = { x, y, z };
    return result;
}

inline void get_triangle(Mesh *mesh, int32 triangle_index, Vec3 triangle[3]) {
    triangle[0] = get_vertex_from_index(mesh, mesh->indices[triangle_index*3]);
    triangle[1] = get_vertex_from_index(mesh, mesh->indices[triangle_index*3 + 1]);
    triangle[2] = get_vertex_from_index(mesh, mesh->indices[triangle_index*3 + 2]);
}

// TODO: we may want to just get rid of the mesh_name_size parameter, and have it just be set to
//       MESH_NAME_MAX_SIZE.
Mesh read_and_load_mesh(Allocator *allocator,
                        String_Buffer filename_buffer,
                        String_Buffer name_buffer) {
    Marker m = begin_region();

    Allocator *global_stack = (Allocator *) &memory.global_stack;
    char *filename = to_char_array(global_stack, filename_buffer);
    File_Data mesh_file = platform_open_and_read_file(global_stack, filename);

    Mesh mesh = Mesh_Loader::load_mesh(mesh_file, allocator);
    mesh.allocator = allocator;
    mesh.filename = filename_buffer;
    mesh.name = name_buffer;
    
    end_region(m);
    
    return mesh;
}

void deallocate(Mesh mesh) {
    delete_string_buffer(mesh.name);
    delete_string_buffer(mesh.filename);
    deallocate(mesh.allocator, mesh.data);
    deallocate(mesh.allocator, mesh.indices);
}
