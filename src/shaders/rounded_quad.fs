#version 330 core

#define TOP_LEFT     0x1u
#define TOP_RIGHT    0x2u
#define BOTTOM_LEFT  0x4u
#define BOTTOM_RIGHT 0x8u

uniform vec2 position;   // top left of the quad
uniform vec2 size;
uniform float corner_radius;
uniform uint corner_flags;

//uniform sampler2D image_texture;
//uniform bool use_color;

in vec2 frag_pos;
in vec4 frag_color;
in vec2 uv;

out vec4 FragColor;

// https://www.youtube.com/watch?v=62-pRVZuS5c
// negative on the inside, positive on the outside
// position is top left of box
float box_sdf(vec2 frag_pos, vec2 position, vec2 half_size) {
    vec2 center = position + half_size;
    
    // abs(frag_pos) moves all points to the bottom right quadrant of the grid who's origin is at the center
    // of the quad
    vec2 d = abs(frag_pos - center) - half_size;

    // length(max(d, 0.0)):
    // zeroes out negative components of the signed distance from box edges to point
    // when we're in the bottom right corner, then we do the full pythagorean theorem
    // when we're in the bottom, to the left of the corner, then we just do the vertical distance from edge to point
    // when we're in the right, we just do the horizontal distance from edge to point

    // min(max(d.x, d.y), 0.0):
    // when we're on the inside, we add the highest component. so if for example, d is (-2, -5), we return
    // -2. this bounds us to the smaller dimension of the size.
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

void main() {
    vec2 half_size = size / 2.0;
    vec2 center = position + half_size;
    vec2 center_to_frag = frag_pos - center;

    uint should_round = 0;
    if (center_to_frag.x < 0.0) {
        if (center_to_frag.y < 0.0) {
            should_round |= (TOP_LEFT & corner_flags);
        } else {
            should_round |= (BOTTOM_LEFT & corner_flags);
        }
    } else {
        if (center_to_frag.y < 0.0) {
            should_round |= (TOP_RIGHT & corner_flags);
        } else {
            should_round |= (BOTTOM_RIGHT & corner_flags);
        }
    }


    float box;
    if (should_round) {
        // shrink the quad by the radius so that we can grow it by the same amount to get rounded corners
        vec2 inner_position = position + vec2(corner_radius);
        vec2 inner_size = size - vec2(corner_radius * 2.0);
        half_size = inner_size / 2.0;
        box = box_sdf(frag_pos, inner_position, half_size) - corner_radius;
    } else {
        box = box_sdf(frag_pos, position, half_size);
    }
    
    #if 0
    float factor = abs(box) / min(size.x / 2.0, size.y / 2.0);
    FragColor = vec4(factor * vec3(1.0f, 1.0f, 1.0f), 1.0f);
    #endif

    float factor = clamp(0.0f, 1.0f, -box + 0.5); // 0.5 so that edges are crisper
    FragColor = vec4(vec3(frag_color), factor);
    
    #if 0
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
        float corner_antialias_end_radius = corner_radius + 0.5f;

        // we use smoothstep instead of discard, since we use MSAA. MSAA only runs the fragment shader once per
        // however many samples we have per pixel. so, if we call discard() on a single pixel, all the samples
        // for that pixel will not be rendered, thus eliminating any anti-aliasing. so we just smoothstep the
        // alpha from the end of the circle to some small distance outside from it to create some look of
        // anti-aliasing. we're basically just feathering the edge to make it look smoother.
        if (bool(corner_flags & TOP_LEFT) && in_top_left) {
            FragColor = vec4(frag_color.xyz, 1.0f - smoothstep(corner_radius, corner_antialias_end_radius, d1));
        } else if (bool(corner_flags & TOP_RIGHT) && in_top_right) {
            FragColor = vec4(frag_color.xyz, 1.0f - smoothstep(corner_radius, corner_antialias_end_radius, d2));
        } else if (bool(corner_flags & BOTTOM_LEFT) && in_bottom_left) {
            FragColor = vec4(frag_color.xyz, 1.0f - smoothstep(corner_radius, corner_antialias_end_radius, d3));
        } else if (bool(corner_flags & BOTTOM_RIGHT) && in_bottom_right) {
            FragColor = vec4(frag_color.xyz, 1.0f - smoothstep(corner_radius, corner_antialias_end_radius, d4));
        } else {
            FragColor = frag_color;
        }
    }
    #endif
}
