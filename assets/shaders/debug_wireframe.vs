#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_uv;

uniform mat4 model_matrix;
uniform mat4 cpv_matrix;
uniform vec4 color;

out vec3 frag_pos;
out vec4 frag_color;
out vec3 normal;

void main() {
    // NOTE: offset the vertex along the normal to prevent z-fighting
    // NOTE: w of vec4 is 0 to ignore the translation of the model matrix
    normal = normalize(vec3(transpose(inverse(model_matrix)) * vec4(vertex_normal, 0.0)));

    vec4 world_position = model_matrix * vec4(pos, 1.0);
    vec4 offset_position = world_position + vec4(normal * 0.001, 0.0);

    frag_pos = vec3(model_matrix * vec4(pos, 1.0));
    gl_Position = cpv_matrix * offset_position;
    frag_color = color;
}
