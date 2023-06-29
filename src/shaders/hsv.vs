#version 330 core

layout (location = 0) in vec2 vertex_pos;
layout (location = 1) in vec2 vertex_uv;
layout (location = 2) in vec4 vertex_color;

uniform mat4 cpv_matrix;

out vec4 frag_color;
out vec2 frag_pos;
out vec2 frag_uv;

void main() {
    gl_Position = cpv_matrix * vec4(int(vertex_pos.x), int(vertex_pos.y), 0.0, 1.0);

    frag_uv = vertex_uv;
    frag_color = vertex_color; // unused
}
