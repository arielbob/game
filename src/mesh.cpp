#include "memory.h"
#include "asset.h"
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
        LABEL,
        OPEN_BRACKET,
        CLOSE_BRACKET,
        STRING
    };

    struct Token {
        Token_Type type;
        String string;
    };

    Token get_token(Tokenizer *tokenizer);
    Token make_token(Token_Type type, char *contents, int32 length);
    bool32 token_text_equals(Token token, char *str);
    bool32 mesh_parse_error(char **error_out, char *error_string);
    
    bool32 parse_real(Tokenizer *tokenizer, real32 *result, char **error);
    bool32 parse_int(Tokenizer *tokenizer, int32 *result, char **error);
    bool32 parse_uint(Tokenizer *tokenizer, uint32 *result, char **error);
    bool32 parse_vec2(Tokenizer *tokenizer, Vec2 *result, char **error);
    bool32 parse_vec3(Tokenizer *tokenizer, Vec3 *result, char **error);
    bool32 parse_vec4(Tokenizer *tokenizer, Vec4 *result, char **error);
    bool32 parse_vec3_int32(Tokenizer *tokenizer, Vec3_int32 *result, char **error);
    bool32 parse_vec3_uint32(Tokenizer *tokenizer, Vec3_uint32 *result, char **error);
    bool32 parse_vec4_int32(Tokenizer *tokenizer, Vec4_int32 *result, char **error);
    bool32 parse_vec4_uint32(Tokenizer *tokenizer, Vec4_uint32 *result, char **error);
    
    bool32 parse_num_vertices(Tokenizer *tokenizer, uint32 *result, char **error);
    bool32 parse_num_triangles(Tokenizer *tokenizer, uint32 *result, char **error);
    bool32 parse_vertex(Tokenizer *tokenizer, Mesh *mesh, uint32 vertex_index, char **error);
    bool32 parse_triangle(Tokenizer *tokenizer, Mesh *mesh, uint32 triangle_index, char **error);
    bool32 parse_bone(Tokenizer *tokenizer, Mesh *mesh, Bone *bone, char **error);
    bool32 parse_skeleton(Tokenizer *tokenizer, Mesh *mesh, char **error);

    bool32 load_mesh(Allocator *allocator, File_Data file_data,
                     Mesh **mesh_result, char **error_out);
}

inline bool32 Mesh_Loader::mesh_parse_error(char **error_out, char *error_string) {
    *error_out = error_string;
    return false;
}

bool32 Mesh_Loader::parse_real(Tokenizer *tokenizer, real32 *result, char **error) {
    Token token = get_token(tokenizer);
    real32 num;
    
    if (token.type == REAL || token.type == INTEGER) {
        bool32 parse_result = string_to_real32(token.string, &num);

        if (!parse_result) {
            return mesh_parse_error(error, "Invalid number value.");
        } else {
            *result = num;
            return true;
        }
    } else {
        return mesh_parse_error(error, "Expected number.");
    }
}

bool32 Mesh_Loader::parse_int(Tokenizer *tokenizer, int32 *result, char **error) {
    Token token = get_token(tokenizer);
    int32 num;
    
    if (token.type == INTEGER) {
        bool32 parse_result = string_to_int32(token.string, &num);

        if (!parse_result) {
            return mesh_parse_error(error, "Invalid integer value.");
        } else {
            *result = num;
            return true;
        }
    } else {
        return mesh_parse_error(error, "Expected integer.");
    }
}

bool32 Mesh_Loader::parse_uint(Tokenizer *tokenizer, uint32 *result, char **error) {
    Token token = get_token(tokenizer);
    uint32 num;
    
    if (token.type == INTEGER) {
        bool32 parse_result = string_to_uint32(token.string, &num);

        if (!parse_result) {
            return mesh_parse_error(error, "Invalid unsigned integer value.");
        } else {
            *result = num;
            return true;
        }
    } else {
        return mesh_parse_error(error, "Expected unsigned integer.");
    }
}

bool32 Mesh_Loader::parse_vec2(Tokenizer *tokenizer, Vec2 *result, char **error) {
    Vec2 v;
    
    for (int i = 0; i < 2; i++) {
        if (!parse_real(tokenizer, &v[i], error)) {
            return mesh_parse_error(error, "Invalid number in Vec2.");
        }
    }

    *result = v;
    return true;
}

bool32 Mesh_Loader::parse_vec3(Tokenizer *tokenizer, Vec3 *result, char **error) {
    Vec3 v;
    
    for (int i = 0; i < 3; i++) {
        if (!parse_real(tokenizer, &v[i], error)) {
            return mesh_parse_error(error, "Invalid number in Vec3.");
        }
    }

    *result = v;
    return true;
}

bool32 Mesh_Loader::parse_vec4(Tokenizer *tokenizer, Vec4 *result, char **error) {
    Vec4 v;
    
    for (int i = 0; i < 4; i++) {
        if (!parse_real(tokenizer, &v[i], error)) {
            return mesh_parse_error(error, "Invalid number in Vec4.");
        }
    }

    *result = v;
    return true;
}

bool32 Mesh_Loader::parse_vec3_int32(Tokenizer *tokenizer, Vec3_int32 *result, char **error) {
    Vec3_int32 v;
    
    for (int i = 0; i < 3; i++) {
        if (!parse_int(tokenizer, &v[i], error)) {
            return mesh_parse_error(error, "Invalid number in Vec3.");
        }
    }

    *result = v;
    return true;
}

bool32 Mesh_Loader::parse_vec3_uint32(Tokenizer *tokenizer, Vec3_uint32 *result, char **error) {
    Vec3_uint32 v;
    
    for (int i = 0; i < 3; i++) {
        if (!parse_uint(tokenizer, &v[i], error)) {
            return mesh_parse_error(error, "Invalid number in Vec3_uint32.");
        }
    }

    *result = v;
    return true;
}

bool32 Mesh_Loader::parse_vec4_int32(Tokenizer *tokenizer, Vec4_int32 *result, char **error) {
    Vec4_int32 v;
    
    for (int i = 0; i < 4; i++) {
        if (!parse_int(tokenizer, &v[i], error)) {
            return mesh_parse_error(error, "Invalid number in Vec4.");
        }
    }

    *result = v;
    return true;
}

bool32 Mesh_Loader::parse_vec4_uint32(Tokenizer *tokenizer, Vec4_uint32 *result, char **error) {
    Vec4_uint32 v;
    
    for (int i = 0; i < 4; i++) {
        if (!parse_uint(tokenizer, &v[i], error)) {
            return mesh_parse_error(error, "Invalid number in Vec4_uint32.");
        }
    }

    *result = v;
    return true;
}

#if 0
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
#endif

inline Mesh_Loader::Token Mesh_Loader::make_token(Token_Type type, char *contents, int32 length) {
    Token token = {
        type,
        make_string(contents, length)
    };
    return token;
}

Mesh_Loader::Token Mesh_Loader::get_token(Tokenizer *tokenizer) {
    Token token = {};

    do {
        consume_leading_whitespace(tokenizer);

        if (is_end(tokenizer)) {
            return make_token(END, NULL, 0);
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

            Token_Type token_type;
            if (has_period) {
                token_type = REAL;
            } else {
                token_type = INTEGER;
            }

            // NOTE: we don't include the null terminator here, and just set bounds using a length member
            token = make_token(token_type, &tokenizer->contents[start], length);
        } else if (tokenizer->current[0] == ';' &&
                   tokenizer->current[1] == ';') {
            increment_tokenizer(tokenizer, 2);
            uint32 start = tokenizer->index;
        
            while (!is_end(tokenizer) &&
                   !is_line_end(tokenizer->current)) {
                increment_tokenizer(tokenizer);
            }

            uint32 length = tokenizer->index - start;
            token = make_token(COMMENT, &tokenizer->contents[start], length);
        } else if (is_letter(tokenizer->current[0])) {
            uint32 start = tokenizer->index;

            increment_tokenizer(tokenizer);
        
            while (!is_end(tokenizer) &&
                   !is_whitespace(tokenizer->current[0])) {

                char current_char = *tokenizer->current;
                if (!(is_letter(current_char) || current_char == '_')) {
                    assert(!"Labels can only contain letters and underscores.");
                }

                increment_tokenizer(tokenizer);
            }

        
            uint32 length = tokenizer->index - start;
            token = make_token(LABEL, &tokenizer->contents[start], length);
        } else if (*tokenizer->current == '{') {
            int32 start = tokenizer->index;
            increment_tokenizer(tokenizer);
            int32 length = tokenizer->index - start;
            token = make_token(OPEN_BRACKET, &tokenizer->contents[start], length);
        } else if (*tokenizer->current == '}') {
            int32 start = tokenizer->index;
            increment_tokenizer(tokenizer);
            int32 length = tokenizer->index - start;
            token = make_token(CLOSE_BRACKET, &tokenizer->contents[start], length);
        } else if (*tokenizer->current == '"') {
            increment_tokenizer(tokenizer);
            // set start after we increment tokenizer so that we don't include the quote in the token string.
            int32 start = tokenizer->index;

            while (!is_end(tokenizer) &&
                   !(*tokenizer->current == '"')) {
                increment_tokenizer(tokenizer);
            }

            if (*tokenizer->current != '"') {
                assert(!"Expected a closing quote.");
            } else {
                int32 length = tokenizer->index - start;
                increment_tokenizer(tokenizer);
                token = make_token(STRING, &tokenizer->contents[start], length);
            }
        } else {
            assert(!"Token type not recognized");
            return {};
        }
    } while (token.type == COMMENT);

    return token;
}

bool32 Mesh_Loader::parse_num_vertices(Tokenizer *tokenizer, uint32 *result, char **error) {
    Token token = get_token(tokenizer);
    if (token.type != INTEGER) {
        return mesh_parse_error(error, "Expected num_vertices integer.");
    }

    bool32 parse_result = string_to_uint32(token.string, result);
    if (parse_result) {
        return true;
    } else {
        return mesh_parse_error(error, "Invalid num_vertices unsigned integer.");
    }
}

bool32 Mesh_Loader::parse_num_triangles(Tokenizer *tokenizer, uint32 *result, char **error) {
    Token token = get_token(tokenizer);
    if (token.type != INTEGER) {
        return mesh_parse_error(error, "Expected num_triangles integer.");
    }

    bool32 parse_result = string_to_uint32(token.string, result);
    if (parse_result) {
        return true;
    } else {
        return mesh_parse_error(error, "Invalid num_triangles unsigned integer.");
    }
}

bool32 Mesh_Loader::parse_vertex(Tokenizer *tokenizer, Mesh *mesh, uint32 vertex_index, char **error) {
    assert(vertex_index < mesh->num_vertices);

    // vertex position
    Token token = get_token(tokenizer);
    if (token.type != LABEL || !string_equals(token.string, "v")) {
        return mesh_parse_error(error, "Expected 'v' label for vertex position.");
    }

    uint8 *data_position = (uint8 *) &mesh->data[vertex_index * mesh->vertex_stride];
    
    Vec3 *position = (Vec3 *) data_position;
    bool32 result = parse_vec3(tokenizer, position, error);
    if (!result) {
        // error message gets set by parse_vec3
        return false;
    }

    data_position += mesh->n_vertex * sizeof(real32);

    mesh->aabb.p_min.x = min(mesh->aabb.p_min.x, position->x);
    mesh->aabb.p_min.y = min(mesh->aabb.p_min.y, position->y);
    mesh->aabb.p_min.z = min(mesh->aabb.p_min.z, position->z);

    mesh->aabb.p_max.x = max(mesh->aabb.p_max.x, position->x);
    mesh->aabb.p_max.y = max(mesh->aabb.p_max.y, position->y);
    mesh->aabb.p_max.z = max(mesh->aabb.p_max.z, position->z);

    // vertex normal
    token = get_token(tokenizer);
    if (token.type != LABEL || !string_equals(token.string, "n")) {
        return mesh_parse_error(error, "Expected 'n' label for vertex normal.");
    }

    Vec3 *normal = (Vec3 *) data_position;
    result = parse_vec3(tokenizer, normal, error);
    if (!result) {
        return false;
    }

    data_position += mesh->n_normal * sizeof(real32);

    // vertex uv
    token = get_token(tokenizer);
    if (token.type != LABEL || !string_equals(token.string, "uv")) {
        return mesh_parse_error(error, "Expected 'uv' label for vertex normal.");
    }

    Vec2 *uv = (Vec2 *) data_position;
    result = parse_vec2(tokenizer, uv, error);
    if (!result) {
        return false;
    }
 
    data_position += mesh->n_uv * sizeof(real32);

    if (mesh->is_skinned) {
        // bone indices
        token = get_token(tokenizer);
        if (token.type != LABEL || !string_equals(token.string, "bi")) {
            return mesh_parse_error(error, "Expected 'bi' label for vertex bone indices.");
        }

        Vec4_int32 *bone_indices = (Vec4_int32 *) data_position;
        result = parse_vec4_int32(tokenizer, bone_indices, error);
        if (!result) {
            return false;
        }

        data_position += mesh->n_bone_indices * sizeof(uint32);

        // bone weights
        token = get_token(tokenizer);
        if (token.type != LABEL || !string_equals(token.string, "bw")) {
            return mesh_parse_error(error, "Expected 'bw' label for vertex bone weights.");
        }

        Vec4 *bone_weights = (Vec4 *) data_position;
        result = parse_vec4(tokenizer, bone_weights, error);
        if (!result) {
            return false;
        }

        assert(fabsf(1.0f -
                     ((*bone_weights)[0] +
                      (*bone_weights)[1] +
                      (*bone_weights)[2] +
                      (*bone_weights)[3])) < EPSILON);

        data_position += mesh->n_bone_weights * sizeof(real32);
    }

    return true;
}

bool32 Mesh_Loader::parse_triangle(Tokenizer *tokenizer, Mesh *mesh, uint32 triangle_index, char **error) {
    assert(triangle_index < mesh->num_triangles);

    Vec3_uint32 *triangle_indices = (Vec3_uint32 *) &mesh->indices[3 * triangle_index];

    return parse_vec3_uint32(tokenizer, triangle_indices, error);
}

bool32 Mesh_Loader::parse_bone(Tokenizer *tokenizer, Mesh *mesh, Bone *bone, char **error) {
    Token token = get_token(tokenizer);
    
    if (!(token.type == LABEL && string_equals(token.string, "bone"))) {
        return mesh_parse_error(error, "Expected bone label for bone block.");
    }

    String bone_name = {};

    token = get_token(tokenizer);
    if (token.type != STRING) {
        return mesh_parse_error(error, "Expected bone name string.");
    } else {
        bone_name = copy(mesh->allocator, token.string);
    }

    token = get_token(tokenizer);
    if (token.type != OPEN_BRACKET) {
        deallocate(bone_name);
        return mesh_parse_error(error, "Expected open bracket for bone block.");
    }

    token = get_token(tokenizer);
    if (!(token.type == LABEL && string_equals(token.string, "inverse_bind"))) {
        deallocate(bone_name);
        return mesh_parse_error(error, "Expected inverse_bind label.");
    }

    // parse inverse bind 3x4 matrix
    Mat4 inverse_bind = make_mat4_identity();

    // the matrix in the file is stored in row-major order, while our Mat4 is
    // in column-major order. (note that row vs. column major is different from
    // whether we use column or row vectors (row*matrix vs matrix*col))
    // they're the same matrix; it's just that the order in memory is different.
    
    // it's easier to read the matrix row by row from our file, but our Mat4
    // is accessed by column. so, we just treat the rows as columns, then transpose.
    // we only go up to 3 since there are only 3 rows (3x4 matrix) because the
    // homogeneous row is useless in skinning.
    for (int32 i = 0; i < 3; i++) {
        if (!parse_vec4(tokenizer, &inverse_bind.values[i], error)) {
            deallocate(bone_name);
            return false;
        }
    }

    int32 parent_index = -1;
    token = get_token(tokenizer);
    if (token.type != CLOSE_BRACKET) {
        // parse parent index
        if (!(token.type == LABEL && string_equals(token.string, "parent"))) {
            deallocate(bone_name);
            return mesh_parse_error(error, "Expected parent label.");
        }

        if (!parse_int(tokenizer, &parent_index, error)) {
            deallocate(bone_name);
            return false;
        }

        // basically try and get put back in the state before this block, i.e.
        // expecting a close bracket.
        token = get_token(tokenizer);
    }

    if (token.type != CLOSE_BRACKET) {
        deallocate(bone_name);
        return mesh_parse_error(error, "Expected close bracket for bone block.");
    }

    bone->name = bone_name;
    bone->parent_index = parent_index;
    bone->model_to_bone_matrix = transpose(inverse_bind);

    return true;
}

bool32 Mesh_Loader::parse_skeleton(Tokenizer *tokenizer, Mesh *mesh, char **error) {
    // parse skeleton_info block
    Token token = get_token(tokenizer);
    if (!(token.type == LABEL && string_equals(token.string, "skeleton"))) {
        return mesh_parse_error(error, "Expected skeleton block.");
    }

    token = get_token(tokenizer);
    if (token.type != OPEN_BRACKET) {
        return mesh_parse_error(error, "Expected open bracket skeleton block.");
    }

    token = get_token(tokenizer);
    if (!(token.type == LABEL && string_equals(token.string, "num_bones"))) {
        return mesh_parse_error(error, "Expected num_bones label.");
    }

    Skeleton skeleton = {};
    skeleton.allocator = mesh->allocator;

    if (!parse_int(tokenizer, &skeleton.num_bones, error)) {
        return false;
    }

    Bone *bones = (Bone *) allocate(skeleton.allocator, sizeof(Bone) * skeleton.num_bones);
    for (int i = 0; i < skeleton.num_bones; i++) {
        if (!parse_bone(tokenizer, mesh, &bones[i], error)) {
            // parse_bone already deallocates the bone itself on error, so no need to do it here
            deallocate(skeleton.allocator, bones);
            return false;
        }
    }

    skeleton.bones = bones;

    mesh->skeleton = (Skeleton *) allocate(mesh->allocator, sizeof(Skeleton));
    *mesh->skeleton = skeleton;

    token = get_token(tokenizer);
    if (token.type != CLOSE_BRACKET) {
        deallocate(mesh->skeleton);
        deallocate(mesh->allocator, mesh->skeleton);
        return mesh_parse_error(error, "Expected close bracket skeleton block.");
    }
    
    return true;
}

bool32 Mesh_Loader::load_mesh(Allocator *allocator, File_Data file_data,
                              Mesh **mesh_result, char **error_out) {
    Tokenizer tokenizer = make_tokenizer(file_data);
    bool32 result;

    Mesh mesh = {};

    mesh.allocator = allocator;
    mesh.n_vertex = 3;
    mesh.n_normal = 3;
    mesh.n_uv = 2;

    mesh.is_skinned = false;
    mesh.n_bone_indices = 0;
    mesh.n_bone_weights = 0;

    // check is_skinned
    Token token = get_token(&tokenizer);
    if (token.type == LABEL && string_equals(token.string, "is_skinned")) {
        int32 is_skinned;
        result = parse_int(&tokenizer, &is_skinned, error_out);

        if (!result) {
            return false;
        } else if (is_skinned != 0 && is_skinned != 1) {
            return mesh_parse_error(error_out, "Invalid value for is_skinned. Expected 0 or 1.");
        }

        mesh.is_skinned = is_skinned;

        if (mesh.is_skinned) {
            mesh.n_bone_indices = MAX_BONE_INDICES;
            mesh.n_bone_weights = MAX_BONE_INDICES;
        }
    } else {
        // if the first thing we see isn't is_skinned, then just go back a token.
        // i.e. not having an is_skinned entry is fine
        increment_tokenizer(&tokenizer, -token.string.length);
    }
    
    mesh.vertex_stride = mesh.n_vertex + mesh.n_normal + mesh.n_uv + mesh.n_bone_indices +
        mesh.n_bone_weights;

    // num_vertices
    result = parse_num_vertices(&tokenizer, &mesh.num_vertices, error_out);
    if (!result) return false;

    mesh.data_size = mesh.num_vertices * mesh.vertex_stride * sizeof(real32);
    mesh.data = (real32 *) allocate(allocator, mesh.data_size);

    // num_triangles
    result = parse_num_triangles(&tokenizer, &mesh.num_triangles, error_out);
    if (!result) {
        deallocate(mesh.allocator, mesh.data);
        return false;
    }

    mesh.indices_size = mesh.num_triangles * 3 * sizeof(uint32);
    mesh.indices = (uint32 *) allocate(allocator, mesh.indices_size);
    
    // parse vertex data
    mesh.aabb.p_min = { INFINITY, INFINITY, INFINITY };
    mesh.aabb.p_max = { -INFINITY, -INFINITY, -INFINITY };

    for (uint32 i = 0; i < mesh.num_vertices; i++) {
        result = parse_vertex(&tokenizer, &mesh, i, error_out);
        if (!result) {
            deallocate(mesh.allocator, mesh.data);
            deallocate(mesh.allocator, mesh.indices);
            return false;
        }
    }

    // parse triangle data
    for (uint32 i = 0; i < mesh.num_triangles; i++) {
        result = parse_triangle(&tokenizer, &mesh, i, error_out);
        if (!result) {
            deallocate(mesh.allocator, mesh.data);
            deallocate(mesh.allocator, mesh.indices);
            return false;
        }
    }

    // parse skeleton data if it's a skinned mesh
    if (mesh.is_skinned) {
        result = parse_skeleton(&tokenizer, &mesh, error_out);
        if (!result) {
            deallocate(mesh.allocator, mesh.data);
            deallocate(mesh.allocator, mesh.indices);
            return false;
        }
    }

    token = get_token(&tokenizer);
    if (token.type != END) {
        deallocate(mesh.allocator, mesh.data);
        deallocate(mesh.allocator, mesh.indices);
        return mesh_parse_error(error_out, "Expected end of mesh file.");
    }

    *mesh_result = (Mesh *) allocate(allocator, sizeof(Mesh));
    **mesh_result = mesh;
    
    return true;
}

#if 0
Mesh Mesh_Loader::load_mesh(File_Data file_data, Allocator *allocator) {
    Tokenizer tokenizer = {};
    tokenizer.current = (char *) file_data.contents;
    tokenizer.index = 0;
    tokenizer.size = file_data.size;

    Token token;

    Mesh mesh = {};

    mesh.allocator = allocator;
    mesh.n_vertex = 3;
    mesh.n_normal = 3;
    mesh.n_uv = 2;

    // TODO: these need to only be set if we're a skinned mesh
    //mesh.n_bone_indices = MAX_BONE_INDICES;
    //mesh.n_bone_weights = MAX_BONE_INDICES;
    
    mesh.vertex_stride = mesh.n_vertex + mesh.n_normal + mesh.n_uv;
    
    Parser_State state = WAITING_FOR_NUM_VERTICES;

    uint32 num_components = 0;
    uint32 current_vertex = 0;
    uint32 indices_so_far = 0;

    AABB aabb = {};
    bool32 aabb_is_initialized = false;

    // we allocate but don't clean up if we error out, but it doesn't really matter.
    // we just assert. we shouldn't have bad mesh data.
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
#endif

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

// this uses the same allocator for everything
void copy(Allocator *allocator, Mesh *mesh_dest, Mesh *mesh_source) {
    *mesh_dest = *mesh_source;

    mesh_dest->name = copy(allocator, mesh_source->name);
    //mesh_dest->filename = copy(allocator, mesh_source->filename);
    //mesh_dest->allocator = allocator;

    mesh_dest->data = (real32 *) allocate(allocator, mesh_source->data_size);
    memcpy(mesh_dest->data, mesh_source->data, mesh_source->data_size);

    mesh_dest->indices = (uint32 *) allocate(allocator, mesh_source->indices_size);
    memcpy(mesh_dest->indices, mesh_source->indices, mesh_source->indices_size);
}
