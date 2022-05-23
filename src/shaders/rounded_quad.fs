#version 330 core

#define TOP_LEFT     0x1
#define TOP_RIGHT    0x2
#define BOTTOM_LEFT  0x4
#define BOTTOM_RIGHT 0x8

uniform vec2 position; // top left of the quad
uniform float width;
uniform float height;
uniform float corner_radius;
uniform uint corner_flags;

//uniform sampler2D image_texture;
//uniform bool use_color;

in vec2 frag_pos;
in vec4 frag_color;
in vec2 uv;

out vec4 FragColor;

void main() {
    float x = position.x;
    float y = position.y;
    vec2 top_left = vec2(x + corner_radius, y + corner_radius);
    vec2 top_right = vec2(x + width - corner_radius, y + corner_radius);
    vec2 bottom_left = vec2(x + corner_radius, y + height - corner_radius);
    vec2 bottom_right = vec2(x + width - corner_radius, y + height - corner_radius);

    bool in_top_left = (frag_pos.x < top_left.x) && (frag_pos.y < top_left.y);
    bool in_top_right = (frag_pos.x > top_right.x) && (frag_pos.y < top_right.y);
    bool in_bottom_left = (frag_pos.x < bottom_left.x) && (frag_pos.y > bottom_left.y);
    bool in_bottom_right = (frag_pos.x > bottom_right.x) && (frag_pos.y > bottom_right.y);
    
    if (!(in_top_left || in_top_right || in_bottom_left || in_bottom_right)) {
        FragColor = frag_color;
    } else {
        float d1 = distance(frag_pos, top_left);
        float d2 = distance(frag_pos, top_right);
        float d3 = distance(frag_pos, bottom_left);
        float d4 = distance(frag_pos, bottom_right);
        if ((bool(corner_flags & TOP_LEFT)     && in_top_left     && (d1 >= corner_radius)) ||
            (bool(corner_flags & TOP_RIGHT)    && in_top_right    && (d2 >= corner_radius)) ||
            (bool(corner_flags & BOTTOM_LEFT)  && in_bottom_left  && (d3 >= corner_radius)) ||
            (bool(corner_flags & BOTTOM_RIGHT) && in_bottom_right && (d4 >= corner_radius))) {
            discard;
        } else {
            FragColor = frag_color;
        }
    }
}
