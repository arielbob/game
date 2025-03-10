#version 330 core

uniform mat4 model_matrix;
uniform mat4 ortho_matrix;
uniform vec4 color;

layout (location = 0) in vec2 pos;

out vec4 frag_color;
out vec2 uv;

void main() {
    gl_Position = ortho_matrix * model_matrix * vec4(pos, 0.0f, 1.0f);
    frag_color = color;
}
