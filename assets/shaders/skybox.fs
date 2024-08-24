#version 440 core

in vec3 texture_sample_dir;

uniform samplerCube cubemap;

out vec4 FragColor;

void main() {
    FragColor = texture(cubemap, texture_sample_dir);
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
