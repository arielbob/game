#version 330 core

uniform mat4 ortho_matrix;
uniform vec4 color;

layout (location = 0) in vec2 window_pos;
layout (location = 1) in vec2 _uv;

out vec4 frag_color;
out vec2 uv;

void main() {
    gl_Position = ortho_matrix * vec4(window_pos, 0.0, 1.0);
    frag_color = color;

    uv = _uv;
}