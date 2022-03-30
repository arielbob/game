#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 vertex_normal;

uniform mat4 model_matrix;
uniform mat4 cpv_matrix;

void main() {
    gl_Position = cpv_matrix * model_matrix * vec4(pos, 1.0);
}
