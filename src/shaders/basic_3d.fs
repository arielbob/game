#version 440 core

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

uniform vec3 material_color;
uniform float gloss;
uniform sampler2D image_texture;
uniform bool use_color_override;

in vec2 uv;
in vec3 frag_pos;
in vec3 normal;

out vec4 FragColor;

float gamma = 2.2;

vec3 calc_point_light(Point_Light point_light,
                      vec3 material_diffuse, vec3 normal, vec3 h, vec3 l, float fragment_to_light_distance) {
    vec3 light_color = pow(vec3(point_light.color), vec3(1.0 / gamma));

    vec3 mat_spec_color = light_color;
    
    vec3 mat_diffuse_color = material_diffuse;

    vec3 light_spec_color = vec3(light_color);
    vec3 light_diffuse_color = vec3(light_color);

    // specular
    vec3 spec_contrib = light_spec_color * mat_spec_color * pow(max(dot(normal, h), 0), gloss);

    // diffuse
    vec3 diffuse_contrib = light_diffuse_color * mat_diffuse_color * max(dot(normal, l), 0);

    float attenuation_factor = 1.0 - ((fragment_to_light_distance - point_light.d_min) /
                                      (point_light.d_max - point_light.d_min));
    attenuation_factor = min(attenuation_factor, 1.0);
    attenuation_factor = max(attenuation_factor, 0.0);

    return (spec_contrib + diffuse_contrib) * attenuation_factor;
}

vec3 calc_sun(vec3 sun_direction, vec3 sun_color,
              vec3 material_diffuse, vec3 normal, vec3 h, vec3 l) {
    vec3 light_color = pow(vec3(sun_color), vec3(1.0 / gamma));

    vec3 mat_spec_color = vec3(1.0f, 1.0f, 1.0f);
    
    vec3 mat_diffuse_color = material_diffuse;

    vec3 light_spec_color = vec3(1.0f, 1.0f, 1.0f);
    vec3 light_diffuse_color = sun_color;

    // specular
    float spec_strength = 0.01f;
    vec3 spec_contrib = vec3(spec_strength) * light_spec_color * mat_spec_color * pow(max(dot(normal, h), 0), gloss);

    // diffuse
    vec3 diffuse_contrib = light_diffuse_color * mat_diffuse_color * max(dot(normal, l), 0);

    return (diffuse_contrib + spec_contrib);
}

void main() {
    // fragment to camera
    vec3 v = normalize(camera_pos - frag_pos);

    vec3 used_color;
    if (use_color_override) {
        used_color = material_color;
    } else {
        used_color = texture(image_texture, uv).xyz;
    }

    used_color = pow(used_color, vec3(1.0 / 2.2));
    vec3 mat_ambient_color = used_color;

    vec3 light_contrib = vec3(0.0);

    // fragment to sun (constant, since sun is directional)
    vec3 sun_direction = normalize(vec3(1.0f, 1.0f, 1.0f));
    vec3 sun_color = vec3(1.0, 1.0, 0.9) * 0.5;
    
    // ambient
    vec3 global_ambient = vec3(0.2, 0.2, 0.2);
    global_ambient = pow(global_ambient, vec3(1.0 / 2.2));
    vec3 ambient_contrib = global_ambient * mat_ambient_color;
    light_contrib += ambient_contrib;

    for (int i = 0; i < num_point_lights; i++) {
        // fragment to light
        vec3 fragment_to_light = point_lights[i].position.xyz - frag_pos;
        float fragment_to_light_distance = length(fragment_to_light);
        vec3 l = fragment_to_light / fragment_to_light_distance;
        // halfway vector
        vec3 h = normalize(v + l);

        Point_Light point_light = point_lights[i];
        light_contrib += calc_point_light(point_light, used_color, normal, h, l, fragment_to_light_distance);
    }

    #if 1
    {
        vec3 l = sun_direction;
        vec3 h = normalize(v + sun_direction);
        light_contrib += calc_sun(sun_direction, sun_color, used_color, normal, h, l);
    }
    #endif

    vec3 gamma_corrected_color = pow(light_contrib, vec3(2.2));
    FragColor = vec4(gamma_corrected_color, 1.0);
}
