#version 330 core

out vec4 FragColor;
  
in vec2 uv;

uniform sampler2D image_texture;

void main() { 
    FragColor = texture(image_texture, uv);
}
