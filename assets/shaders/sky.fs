#version 440 core

in vec3 texture_dir;

uniform sampler2D cubemap;

void main() {
    FragColor = texture(cubemap, texture_dir);
}
