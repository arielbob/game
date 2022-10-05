#version 330 core

uniform sampler2D image_texture;
uniform bool use_texture;

in vec4 frag_color;
in vec2 frag_uv;

out vec4 FragColor;

void main() {
    if (use_texture) {
        FragColor = texture(image_texture, frag_uv);
    } else {
        FragColor = frag_color;
    }
}
