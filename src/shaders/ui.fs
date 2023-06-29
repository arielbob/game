#version 330 core

uniform sampler2D image_texture;
uniform bool is_text;
uniform bool use_texture;

in vec4 frag_color;
in vec2 frag_uv;

out vec4 FragColor;

void main() {
    if (use_texture) {
        if (is_text) {
            FragColor = vec4(frag_color.xyz, texture(image_texture, frag_uv).a * frag_color.a);
            //FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        } else {
            FragColor = texture(image_texture, frag_uv);    
        }
    } else {
        FragColor = frag_color;
    }
}
