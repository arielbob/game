#version 330 core

uniform mat4 cpv_matrix;
uniform vec3 color;

layout (location = 0) in vec2 window_pos;
layout (location = 1) in vec2 _uv;

out vec4 frag_color;
out vec2 uv;

void main() {
    gl_Position = cpv_matrix * vec4(window_pos, 0.0, 1.0);
    frag_color = vec4(color, 1.0f);

    uv = _uv;
}
