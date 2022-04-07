#version 330 core

uniform sampler2D image_texture;
uniform bool use_color;

in vec4 frag_color;
in vec2 uv;

out vec4 FragColor;

void main() {
    if (use_color) {
        FragColor = frag_color;
    } else {
        FragColor = texture(image_texture, uv);
    }
}
