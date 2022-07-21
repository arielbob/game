#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 vertex_uv;

uniform mat4 model_matrix;
uniform mat4 cpv_matrix;

out vec2 uv;

void main() {
    gl_Position = cpv_matrix * model_matrix * vec4(pos, 1.0);
    uv = vertex_uv;
}
