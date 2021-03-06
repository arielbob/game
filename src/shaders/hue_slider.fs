#version 330 core

in vec4 frag_color;
in vec2 uv;

out vec4 FragColor;

void main() {
    vec3 colors[6] = {
        vec3(1.0, 0.0, 0.0),
        vec3(1.0, 1.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, 1.0, 1.0),
        vec3(0.0, 0.0, 1.0),
        vec3(1.0, 0.0, 1.0)
    };
    
    float segment_percentage = mod(uv.y * 6.0, 1.0);
    //vec3 color = vec3(uv.y);
    //vec3 color = vec3(segment_percentage);
    int segment = int(uv.y * 6.0);
    
    vec3 color1 = colors[segment];
    vec3 color2 = colors[(segment + 1) % 6];
    
    vec3 hue = mix(color1, color2, segment_percentage);
    vec3 color = hue;

    FragColor = vec4(color, 1.0f);
}
