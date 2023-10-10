#version 440 core

#define PI 3.1415926538

// size of this struct is 16 + 16 + 4 + 4 + 8
// final 8 bytes is to make final size a multiple of 16 (i.e. multiple of size of a vec4)
struct Point_Light {
    vec4 position;
    vec4 color;
    float intensity;
    // this attenuation formula isn't very physically accurate, but it allows us to easily bound the light.
    // it also allows for more intuitive artistic control.
    float falloff_start;
    float falloff_end;
};

struct Sun_Light {
    vec4 color;
    vec4 direction; // vector pointing the way the sun is pointing (calculated by us on cpu)
};

layout (std140) uniform shader_globals {
    //                                 aligned offset
    int num_point_lights;           // 0
    Point_Light point_lights[16];   // 16
    int num_sun_lights;             // 64
    Sun_Light sun_lights[16];       // 80
};

uniform vec3 camera_pos;

uniform bool use_albedo_texture;
uniform bool use_metalness_texture;
uniform bool use_roughness_texture;

uniform vec3 u_albedo_color;
uniform float u_metalness;
uniform float u_roughness;
uniform float ao;

uniform sampler2D albedo_texture;
uniform sampler2D metalness_texture;
uniform sampler2D roughness_texture;

in vec2 uv;
in vec3 frag_pos;
in vec3 normal;

out vec4 FragColor;

float gamma = 2.2;

float normal_distribution_ggx(vec3 n, vec3 h, float roughness) {
    float a = roughness*roughness;
    float a_squared = a*a;

    float n_dot_h = max(dot(n, h), 0.0);
    float n_dot_h_squared = n_dot_h*n_dot_h;

    float numerator = a_squared;
    float denom = n_dot_h_squared*(a_squared - 1.0) + 1.0;
    denom = PI * denom*denom;

    return numerator / denom;
}

float geometry_shlick_ggx(float n_dot_v, float roughness) {
    float r = roughness + 1.0;
    float k = r*r / 8.0; // k for direct lighting

    float numerator = n_dot_v;
    float denom = n_dot_v*(1.0 - k) + k;

    return numerator / denom;
}

float geometry_smith(vec3 n, vec3 v, vec3 l, float roughness) {
    float n_dot_v = max(dot(n, v), 0.0);
    float n_dot_l = max(dot(n, l), 0.0);

    float ggx1 = geometry_shlick_ggx(n_dot_v, roughness);
    float ggx2 = geometry_shlick_ggx(n_dot_l, roughness);

    return ggx1 * ggx1;
}

// as h_dot_v approaches 0, more spec is shown.
// if the light and camera are right above the surface, h_dot_v will be 0, i.e. least amount of spec.
// if the light and camera are opposite each other, this is when spec will be the most. so, unlike
// regular phong spec calculations, the spec is not the greatest when the view vector is closest
// with the reflection vector of the light (or when the halfway vector is closest to the normal).
vec3 fresnel_schlick(float h_dot_v, vec3 f0) {
    return f0 + (1.0 - f0)*pow(clamp(1.0 - h_dot_v, 0.0, 1.0), 5.0);
}

void main() {
    // fragment to camera
    vec3 v = normalize(camera_pos - frag_pos);
    vec3 n = normalize(normal);
    
    vec3 albedo;
    float metalness;
    float roughness;

#if 1
    if (use_albedo_texture) {
        albedo = texture(albedo_texture, uv).xyz;
    } else {
        albedo = u_albedo_color;
    }

    if (use_metalness_texture) {
        metalness = texture(metalness_texture, uv).x;
    } else {
        metalness = u_metalness;
    }
    
    if (use_roughness_texture) {
        roughness = texture(roughness_texture, uv).x;
    } else {
        roughness = u_roughness;
    }
#endif
    
    albedo    = pow(albedo, vec3(1.0 / 2.2));
    metalness = pow(metalness, 1.0 / 2.2);
    roughness = pow(roughness, 1.0 / 2.2);

    vec3 light_out = vec3(0.0);

    for (int i = 0; i < num_point_lights; i++) {
        // fragment to light
        vec3 fragment_to_light = point_lights[i].position.xyz - frag_pos;
        float fragment_to_light_distance = length(fragment_to_light);
        vec3 l = fragment_to_light / fragment_to_light_distance;
        // halfway vector
        vec3 h = normalize(v + l);
        
        Point_Light point_light = point_lights[i];
        //float attenuation = 1.0 / (fragment_to_light_distance * fragment_to_light_distance);

        float attenuation = 1.0 - ((fragment_to_light_distance - point_light.falloff_start) /
                                   (point_light.falloff_end - point_light.falloff_start));
        attenuation = clamp(attenuation, 0.0f, 1.0f);
        //attenuation = max(attenuation, 0.0f);

        // this is L_i, and for point lights, it's always the same
        vec3 light_color = pow(vec3(point_light.color) * max(0.0, point_light.intensity), vec3(1.0 / gamma));
        vec3 radiance = attenuation * light_color;

        vec3 f0 = vec3(0.04);
        f0 = mix(f0, albedo, metalness);
        // i'm pretty sure we don't need to do max(dot(h, v), 0.0) here
        vec3 fresnel = fresnel_schlick(dot(h, v), f0);
        float ndf = normal_distribution_ggx(n, h, roughness);
        float geometry = geometry_smith(n, v, l, roughness);

        // cook-torrance specular brdf
        float v_dot_n = max(dot(v, n), 0.0);
        float n_dot_l = max(dot(n, l), 0.0);
        // add 0.0001 to prevent divide by 0
        vec3 specular = (ndf * fresnel * geometry) / ((4.0 * v_dot_n * n_dot_l) + 0.0001);

        vec3 k_specular = fresnel;
        vec3 k_diffuse = vec3(1.0) - k_specular;
        k_diffuse *= (1.0 - metalness);
        
        // we don't use k_specular here, since fresnel == k_specular and is already included
        // in the specular term.
        light_out += ((k_diffuse * albedo / PI) + specular) * radiance * n_dot_l;
    }

    vec3 ambient = vec3(0.5) * albedo * ao;
    vec3 color = ambient + light_out;

    color = color / (color + vec3(1.0)); // reinhard tone mapping
    color = pow(color, vec3(2.2));       // gamma correction
    
    FragColor = vec4(color, 1.0);
    //FragColor = vec4(num_point_lights, 0.0, 0.0, 1.0);
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
