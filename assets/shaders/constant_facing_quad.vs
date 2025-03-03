#version 330 core

uniform mat4 perspective_clip_matrix;
uniform float side_length;
uniform vec3 view_space_center;

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 vertex_uv;

out vec2 uv;

void main() {
    vec4 view_space_vertex_position = vec4(view_space_center + vec3(side_length*pos, 0.0), 1.0f);
    gl_Position = perspective_clip_matrix * view_space_vertex_position;

    uv = vertex_uv;
}
