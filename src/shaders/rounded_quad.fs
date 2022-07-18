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

void main() {
    vec2 texture_size = textureSize(alpha_texture, 0);
    // flip y, so that it matches uv's coordinate space (y increases going up in uv-space)
    vec2 screen_position = vec2(frag_pos.x, -(frag_pos.y - texture_size.y));
    float alpha = (texture(alpha_texture, screen_position / texture_size)).r;
    
    vec2 half_size = size / 2.0;
    vec2 center = position + half_size;
    vec2 center_to_frag = frag_pos - center;

    float radius = 0.0;
    if (center_to_frag.x < 0.0) {
        if (center_to_frag.y < 0.0) {
            if (TOP_LEFT & corner_flags) {
                radius = corner_radius;
            }
        } else {
            if (BOTTOM_LEFT & corner_flags) {
                radius = corner_radius;
            }
        }
    } else {
        if (center_to_frag.y < 0.0) {
            if (TOP_RIGHT & corner_flags) {
                radius = corner_radius;
            }
        } else {
            if (BOTTOM_RIGHT & corner_flags) {
                radius = corner_radius;
            }
        }
    }

    float side_border_width = 0.0;
    if (abs(center_to_frag.x) > (half_size.x - border_width - radius)) {
        if (center_to_frag.x < 0.0) {
            if (BORDER_LEFT & border_side_flags) {
                side_border_width = border_width;
            }
        } else {
            if (BORDER_RIGHT & border_side_flags) {
                side_border_width = border_width;
            }
        }
    }

    if (abs(center_to_frag.y) > (half_size.y - border_width - radius)) {
        if (center_to_frag.y < 0.0) {
            if (BORDER_TOP & border_side_flags) {
                side_border_width = border_width;
            }
        } else {
            if (BORDER_BOTTOM & border_side_flags) {
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
    box_d = -box_d + 0.5; // add 0.5 so that edges are crisper
    
    float border_factor = smoothstep(side_border_width, side_border_width + 1.0, box_d);

    float factor = clamp(0.0, 1.0, box_d);

    if (is_alpha_pass) {
        // for some reason, the alpha mask framebuffer still has alpha even though its format is GL_RED, so we keep
        // alpha 1.0 here even though it should ignore it. and technically, we can, since we only use the red
        // channel, but when we draw the texture for debugging purposes using the alpha mask framebuffer, if we set
        // alpha to 0.0, you won't see anything.
        // this might have something to do with premultiplied alpha? i forget how that works.
        FragColor = vec4(factor, 0.0f, 0.0f, 1.0); 
    } else {
        FragColor = vec4(mix(vec3(border_color), vec3(frag_color), border_factor), alpha);
    }
}
