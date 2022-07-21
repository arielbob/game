#version 330 core

uniform mat4 cpv_matrix;
uniform vec4 color;
uniform bool has_shadow;
uniform float shadow_offset;
uniform vec4 shadow_color;

layout (location = 0) in vec2 base_pos;
layout (location = 1) in vec2 base_uv;

layout (location = 2) in vec2 glyph_position;
layout (location = 3) in vec2 glyph_size;
layout (location = 4) in vec2 uv_position;
layout (location = 5) in vec2 uv_size;

out vec4 frag_color;
out vec2 uv;
out int instance_id;

void main() {
#if 0
    float y_pos;
    if (has_shadow && gl_InstanceID == 0) {
        frag_color = shadow_color;
        y_pos = window_pos.y + shadow_offset;
    } else {
        frag_color = color;
        y_pos = window_pos.y;
    }
#endif

    frag_color = color;

    vec2 vertex_window_pos = vec2(base_pos.x*glyph_size.x, base_pos.y*glyph_size.y) + glyph_position;
    vec2 vertex_uv         = vec2(base_uv.x*uv_size.x, base_uv.y*uv_size.y) + uv_position;
    
    gl_Position = cpv_matrix * vec4(vertex_window_pos.x, vertex_window_pos.y, 0.0, 1.0);
    uv = vertex_uv;
}
