#version 330 core

#define NUM_BONES 16 // this is just a test value for the number of bones in the skeleton

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_uv;
layout (location = 3) in ivec4 bone_indices;
layout (location = 4) in vec4 bone_weights;

uniform bool is_skinned;

/*
- we have animation data
- when drawing an entity, we sample the animation data at whatever time
- we get the matrices for all the bones for that sample
- we send the matrices to gl_draw_mesh()
 */

// model->bone->pose->model matrices
// calculated like this: B * (B->P) * (B)^-1
// where B is the bone->model space matrix, i.e. the transform that positions and orients
// the bone on the mesh.
// B^-1 is sometimes called the inverse bind matrix, which does the opposite of B. multiplying
// a vertex of the mesh in its bind pose transforms it from model space to bone-space.
// now that the vertex is in bone-space, we can apply the bone's local transform by muiltiplying
// it by the bone to pose-space matrix (B->P).
// now we can convert it back to model-space by multiplying by B. and we get the final model-space
// position.
uniform mat4 bone_matrices[NUM_BONES];

uniform mat4 model_matrix;
uniform mat4 cpv_matrix;

out vec3 frag_pos;
out vec3 normal;
out vec2 uv;
out vec3 vertex_color;

void main() {
    // the vertices are in model-space and are positioned in the bind pose.
    // the bones in the bind pose are in model-space.
    // then we can just take the bone-space to pose-space matrix and multiply the vertex
    // by that, and the vertex will be in pose-space.

    // model_matrix converts from model-space to world-space
    mat4 model_to_world_matrix = model_matrix;
    vec4 skinned_pos = vec4(pos, 1.0);
    mat4 weighted_skin_matrix = mat4(1.0);
    
    if (is_skinned) {
        skinned_pos = vec4(0.0);
        weighted_skin_matrix = mat4(0.0);
        // we assume the bone_weights add to 1.. if they don't we'll end up with an empty skinning
        // matrix, and thus empty model_matrix as well
        for (int i = 0; i < 4; i++) {
            weighted_skin_matrix += bone_matrices[bone_indices[i]] * bone_weights[i];
        }
        skinned_pos = weighted_skin_matrix * vec4(pos, 1.0);
    }
    
    frag_pos = vec3(model_matrix * skinned_pos);
    gl_Position = cpv_matrix * model_matrix * skinned_pos;

    #if 0
    vertex_color = vec3(bone_indices[0], bone_indices[0], bone_indices[0]);
    if (is_skinned) {
        // TODO: the top vertices' bone_indices[0] are not 1 for some reason?????
        if (bone_indices[0] == 0) {
            vertex_color = vec3(1.0, 0.0, 0.0);
        } else {
            vertex_color = vec3(0.0, 1.0, 0.0);
        }
    } else {
        vertex_color = vec3(0.0, 0.0, 1.0);
    }
    #endif
    
    // NOTE: w of vec4 is 0 to ignore the translation of the model matrix
    normal = normalize(vec3(transpose(inverse(model_matrix * weighted_skin_matrix)) * vec4(vertex_normal, 0.0)));
    uv = vertex_uv;
}
