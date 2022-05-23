#version 330 core

out vec4 FragColor;
  
in vec2 uv;

uniform sampler2DMS image_texture;
uniform int num_samples;

vec4 texture_multisample(sampler2DMS sampler, ivec2 coord) {
    vec4 color = vec4(0.0);

    for (int i = 0; i < num_samples; i++) {
        color += texelFetch(sampler, coord, i);
    }

    color /= float(num_samples);

    return color;
}

void main() {
    ivec2 texture_size = textureSize(image_texture);
    ivec2 texture_coord = ivec2(uv * texture_size);

    FragColor = texture_multisample(image_texture, texture_coord);
}
