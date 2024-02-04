#version 330 core

#define NUM_BONES 16 // this is just a test value for the number of bones in the skeleton

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_uv;
layout (location = 3) in ivec4 bone_indices;
layout (location = 4) in vec4 bone_weights;

uniform bool is_skinned;
uniform mat4 bind_to_pose_matrices[NUM_BONES];
uniform mat4 model_matrix;
uniform mat4 cpv_matrix;

out vec3 frag_pos;
out vec3 normal;
out vec2 uv;

void main() {
    // the vertices are in model-space and are positioned in the bind pose.
    // the bones in the bind pose are in model-space.
    // then we can just take the bone-space to pose-space matrix and multiply the vertex
    // by that, and the vertex will be in pose-space.

    // model_matrix converts from model-space to world-space
    mat4 model_to_world_matrix = model_matrix;
    
    if (is_skinned) {
        mat4 pose_matrix = mat4(0.0);
        for (int i = 0; i < 4; i++) {
            pose_matrix += bind_to_pose_matrices[i] * bone_weights[i];
        }

        // pose first (vertices are still in model-space), then convert from
        // model-space to world-space.
        model_to_world_matrix = model_matrix * pose_matrix;
    }
    
    frag_pos = vec3(model_to_world_matrix * vec4(pos, 1.0));
    gl_Position = cpv_matrix * model_to_world_matrix * vec4(pos, 1.0);

    // NOTE: w of vec4 is 0 to ignore the translation of the model matrix
    normal = normalize(vec3(transpose(inverse(model_to_world_matrix)) * vec4(vertex_normal, 0.0)));
    uv = vertex_uv;
}
