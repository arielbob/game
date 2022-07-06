#version 440 core

#define PI 3.1415926538

struct Point_Light {
    vec4 position;
    vec4 color;
    // this attenuation formula isn't very physically accurate, but it allows us to easily bound the light.
    // it also allows for more intuitive artistic control.
    float d_min;
    float d_max;
};

layout (std140) uniform shader_globals {
    //                                 aligned offset
    int num_point_lights;           // 0
    Point_Light point_lights[16];   // 16
};

uniform vec3 camera_pos;

uniform vec3 albedo_color;
uniform float metallic;
uniform float roughness;
//uniform float ao;

uniform sampler2D albedo_texture;
uniform bool use_color_override;

in vec2 uv;
in vec3 frag_pos;
in vec3 normal;

out vec4 FragColor;

float gamma = 2.2;

vec3 normal_distribution_ggx(vec3 n, vec3 h, float roughness) {
    float a = roughness*roughness;
    float a_squared = a*a;

    float n_dot_h = max(dot(n, h), 0.0);
    float n_dot_h_squared = n_dot_h*n_dot_h;

    float numerator = a_squared;
    float denom = n_dot_h_squared*(a_squared - 1.0) + 1.0;
    denom = PI * denom*denom;

    float ndf = numerator / denom;
    return ndf;
}

// as h_dot_v approaches 0, more spec is shown.
// if the light and camera are right above the surface, h_dot_v will be 0, i.e. least amount of spec.
// if the light and camera are opposite each other, this is when spec will be the most. so, unlike
// regular phong spec calculations, the spec is not the greatest when the view vector is closest
// with the reflection vector of the light (or when the halfway vector is closest to the normal).
vec3 fresnel_schlick(float h_dot_v, vec3 f0) {
    return f0 + (1.0 - f0)*pow(clamp(1.0 - h_dot_v, 0.0, 1.0), 5.0);
}

// TODO: test on non-intel gpu
void main() {
    // fragment to camera
    vec3 v = normalize(camera_pos - frag_pos);
    vec3 n = normalize(normal);
    
    vec3 albedo;
    if (use_color_override) {
        albedo = albedo_color;
    } else {
        albedo = texture(albedo_texture, uv).xyz;
    }

    albedo = pow(albedo, vec3(1.0 / 2.2));

    vec3 light_out = vec3(0.0);

    for (int i = 0; i < num_point_lights; i++) {
        // fragment to light
        vec3 fragment_to_light = point_lights[i].position.xyz - frag_pos;
        float fragment_to_light_distance = length(fragment_to_light);
        vec3 l = fragment_to_light / fragment_to_light_distance;
        // halfway vector
        vec3 h = normalize(v + l);
        float n_dot_l = max(dot(n, l), 0.0);
        
        Point_Light point_light = point_lights[i];
        float attenuation = 1.0 / (fragment_to_light_distance * fragment_to_light_distance);

        vec3 light_color = pow(vec3(point_light.color), vec3(1.0 / gamma));
        vec3 radiance = attenuation * light_color;

        vec3 f0 = vec3(0.04);
        f0 = mix(f0, albedo, metallic);
        // i'm pretty sure we don't need to do max(dot(h, v), 0.0) here
        vec3 fresnel = fresnel_schlick(dot(h, v), f0);
        vec3 ndf = normal_distribution_ggx(n, h, roughness);
        
        vec3 brdf = (fresnel * ndf) / (4.0 * dot(v, n) * n_dot_l);
        
        //light_out += radiance * fresnel * n_dot_l;
        light_out += brdf * radiance * n_dot_l;
    }

    vec3 gamma_corrected_color = pow(light_out, vec3(2.2));
    FragColor = vec4(gamma_corrected_color, 1.0);
}
