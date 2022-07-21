#version 330 core

in vec2 uv;

uniform bool use_color_override;
uniform vec4 color;
uniform sampler2D image_texture;

out vec4 FragColor;

void main() {
    if (use_color_override) {
        FragColor = color;
    } else {
        FragColor = texture(image_texture, uv);
    }
}
