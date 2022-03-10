#version 330 core

uniform sampler2D image_texture;

in vec4 frag_color;
in vec2 uv;

out vec4 FragColor;

void main() {
    FragColor = vec4(frag_color.xyz, texture(image_texture, uv).a);
}
