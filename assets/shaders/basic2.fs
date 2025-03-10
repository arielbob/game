#version 330 core

uniform sampler2D alpha_texture;
uniform sampler2D image_texture;
uniform bool use_color;
uniform bool has_alpha;

in vec2 frag_pos;
in vec4 frag_color;
in vec2 uv;

out vec4 FragColor;

float get_alpha_from_screen_position() {
    vec2 texture_size = textureSize(alpha_texture, 0);
    // flip y, so that it matches uv's coordinate space (y increases going up in uv-space)
    vec2 screen_position = vec2(frag_pos.x, -(frag_pos.y - texture_size.y));
    float alpha = (texture(alpha_texture, screen_position / texture_size)).r;
    return alpha;
}

void main() {
    if (use_color) {
        FragColor = frag_color;
    } else {
        FragColor = texture(image_texture, uv);
    }

    if (has_alpha) {
        FragColor.a *= get_alpha_from_screen_position();
    }
}
