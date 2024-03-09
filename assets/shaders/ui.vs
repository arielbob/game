#version 330 core

layout (location = 0) in vec2 vertex_pos;
layout (location = 1) in vec2 vertex_uv;
layout (location = 2) in vec4 vertex_color;

uniform mat4 cpv_matrix;
uniform bool use_texture;

out vec4 frag_color;
out vec2 frag_pos;
out vec2 frag_uv;

void main() {
    // we round the x and y positions so that we are pixel aligned and don't end up doing any
    // for textures that should fit perfectly into some quad (for example, the 15x15 arrow texture
    // in the 15x15 quad for dropdowns).
    if (use_texture) {
        gl_Position = cpv_matrix * vec4(int(vertex_pos.x), int(vertex_pos.y), 0.0, 1.0);
    } else {
        gl_Position = cpv_matrix * vec4(vertex_pos.x, vertex_pos.y, 0.0, 1.0);
    }

    frag_uv = vertex_uv;
    frag_color = vertex_color;
}
