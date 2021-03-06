#version 330 core

uniform float hue_degrees;

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
    
    int segment = int(hue_degrees / 60.0) % 6;
    // this doesn't work with opengl 3.3, modf and fract are only in versions >= 4, i think
    //float segment_percentage = modf(hue_degrees, 60);
    float segment_percentage = (hue_degrees / 60.0) - int(hue_degrees / 60.0);

    vec3 color1 = colors[segment];
    vec3 color2 = colors[(segment + 1) % 6];
    
    // NOTE: for some reason this can be off by 1 from paint.net, but with google's color picker
    //       this value gives the correct value. for example, with hue_degrees = 50, paint.net says
    //       that the color has hue of 49, but google's color picker says hue is 50. not sure which
    //       one is correct, but i think it's fine.
    vec3 hue = mix(color1, color2, segment_percentage);
    vec3 white = vec3(1.0, 1.0, 1.0);
    vec3 black = vec3(0.0);
    vec3 color = mix(white, hue, uv.x);
    color = mix(black, color, uv.y);

    FragColor = vec4(color, 1.0f);
}
