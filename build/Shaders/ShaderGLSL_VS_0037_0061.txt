#version 330 core

layout (location = 0) in vec3 pos;

uniform vec4 color;
uniform mat4 model_matrix;
uniform mat4 cpv_matrix;

out vec3 frag_pos;
out vec4 frag_color;

void main() {
    frag_color = color;
    frag_pos = vec3(model_matrix * vec4(pos, 1.0));
    gl_Position = cpv_matrix * model_matrix * vec4(pos, 1.0);
}
