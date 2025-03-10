#version 330 core

#define TOP_LEFT     0x1u
#define TOP_RIGHT    0x2u
#define BOTTOM_LEFT  0x4u
#define BOTTOM_RIGHT 0x8u

#define BORDER_LEFT   0x1u
#define BORDER_RIGHT  0x2u
#define BORDER_BOTTOM 0x4u
#define BORDER_TOP    0x8u

uniform vec2 position; // top left of the quad
uniform vec2 size;
uniform float corner_radius;
uniform uint corner_flags;
uniform float border_width;
uniform uint border_side_flags;
uniform vec4 border_color;
uniform bool is_alpha_pass;
uniform bool has_alpha;

uniform sampler2D alpha_texture;
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

float get_alpha_from_screen_position() {
    vec2 texture_size = textureSize(alpha_texture, 0);
    // flip y, so that it matches uv's coordinate space (y increases going up in uv-space)
    vec2 screen_position = vec2(frag_pos.x, -(frag_pos.y - texture_size.y));
    float alpha = (texture(alpha_texture, screen_position / texture_size)).r;
    return alpha;
}

void main() {
    float alpha = 1.0;
    if (has_alpha) {
        alpha = get_alpha_from_screen_position();
    }
    
    vec2 half_size = size / 2.0;
    vec2 center = position + half_size;
    vec2 center_to_frag = frag_pos - center;

    float radius = 0.0;
    if (center_to_frag.x < 0.0) {
        if (center_to_frag.y < 0.0) {
            if (bool(TOP_LEFT & corner_flags)) {
                radius = corner_radius;
            }
        } else {
            if (bool(BOTTOM_LEFT & corner_flags)) {
                radius = corner_radius;
            }
        }
    } else {
        if (center_to_frag.y < 0.0) {
            if (bool(TOP_RIGHT & corner_flags)) {
                radius = corner_radius;
            }
        } else {
            if (bool(BOTTOM_RIGHT & corner_flags)) {
                radius = corner_radius;
            }
        }
    }

    float side_border_width = 0.0;
    if (abs(center_to_frag.x) > (half_size.x - border_width - radius)) {
        if (center_to_frag.x < 0.0) {
            if (bool(BORDER_LEFT & border_side_flags)) {
                side_border_width = border_width;
            }
        } else {
            if (bool(BORDER_RIGHT & border_side_flags)) {
                side_border_width = border_width;
            }
        }
    }

    if (abs(center_to_frag.y) > (half_size.y - border_width - radius)) {
        if (center_to_frag.y < 0.0) {
            if (bool(BORDER_TOP & border_side_flags)) {
                side_border_width = border_width;
            }
        } else {
            if (bool(BORDER_BOTTOM & border_side_flags)) {
                side_border_width = border_width;
            }
        }
    }

    float shrink_amount = radius;

    // uncomment this if you want the inner border to round around rounded corners. a consequence of this is that
    // the outer radius will have to become larger to compensate, and thus the outer radius will not match the
    // passed in corner_radius. this is just due to the fact that as you go inwards into the sdf, the rectangle
    // becomes more straight. so if you have a large border_width compared to your corner_radius, without compensating,
    // the inner border will have straight corners. if you instead go outwards from the sdf, the rectangle
    // becomes rounder. that's why when we shrink by the side border width and then expand by border_width + radius,
    // the outer corners become rounder.
    //
    // in the current way, with this commented out, as long as your border width is small compared to your radius,
    // the border will be rounded.
    #if 0
    if (radius > 0.0) {
        // when there are no rounded corners, we don't shrink/grow the sdf, since we don't want the corner to round
        // at all. we shrink by side_border_width when there is a rounded corner because we want the border to
        // also be rounded. we do that be shrinking by the radius + side_border_width, then growing twice. that
        // way, the border is rounded along with the corner.
        shrink_amount += side_border_width;
    }
    #endif
    
    vec2 inner_position = position + vec2(shrink_amount);
    vec2 inner_size = size - vec2(shrink_amount * 2.0);
    
    half_size = inner_size / 2.0;
    float box_d = box_sdf(frag_pos, inner_position, half_size) - shrink_amount;

    if (is_alpha_pass) {
        box_d += side_border_width; // shrink the sdf by border so that we mask the inner area of the box
    }
    
    // take negative so that it grows as you move further inwards (do this so that we can just clamp to get it to be 0/1
    // for outside/inside)
    // adding 0.5 here, makes it so the box grows. this basically just clips off the edges a bit, making the edges
    // crisper
    box_d = -box_d + 0.5;
    
    //float factor = clamp(0.0, 1.0, box_d);
    // for some reason, clamp doesn't work with negative numbers on my laptop's intel integrated graphics,
    // so i was getting a negative alpha value, which lead to colors getting inverted when under a transparent
    // area. so, we replace it with what is should be doing, and it works.
    float factor = min(max(0.0, box_d), 1.0);

    if (is_alpha_pass) {
        // make sure to turn off GL_BLEND when drawing to the alpha mask or else the alpha value here will
        // affect the output
        FragColor = vec4(factor * alpha, 0.0, 0.0, 0.0);
    } else {
        // box_d is in pixels. border_factor is between 0 and 1 from the inner edge of the border to the inner edge
        // of the border + some small threshold. then we use this border_factor to figure out what to color the border.
        // if it's zero, meaning it's between the inner edge of the border and the outside border of the window, then
        // it's the border color. if it's one, then it's the window color. the threshold amount changes how wide the
        // change is between border color and the window color.
        
        // we add 0.5 to where the inner edge starts to push it in a bit. this is just so that if we have a one pixel
        // wide border, we don't get half the transparency of the border since the border ends up on the exact
        // edge of the quad. this way if we have a 1 pixel border, the color will be exactly the border color passed in.
        float border_factor = smoothstep(side_border_width + 0.5, side_border_width + 1.0, box_d);
        FragColor = vec4(mix(vec3(border_color), vec3(frag_color), border_factor), factor * alpha);
    }
}
