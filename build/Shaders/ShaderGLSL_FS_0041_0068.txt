#version 330 core

uniform sampler2D image_texture;

in vec2 uv;

out vec4 FragColor;

void main() {
    FragColor = texture(image_texture, uv);
}
