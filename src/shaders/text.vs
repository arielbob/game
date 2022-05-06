#version 330 core

uniform mat4 cpv_matrix;
uniform vec4 color;
uniform bool has_shadow;
uniform float shadow_offset;
uniform vec4 shadow_color;

layout (location = 0) in vec2 window_pos;
layout (location = 1) in vec2 _uv;

out vec4 frag_color;
out vec2 uv;
out int instance_id;

void main() {
    float y_pos;
    if (has_shadow && gl_InstanceID == 0) {
        frag_color = shadow_color;
        y_pos = window_pos.y + shadow_offset;
    } else {
        frag_color = color;
        y_pos = window_pos.y;
    }

    gl_Position = cpv_matrix * vec4(window_pos.x, y_pos, 0.0, 1.0);
    
    uv = _uv;
}
