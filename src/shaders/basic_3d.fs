#version 440 core

struct Point_Light {
    vec4 position;
    vec4 color;
};

layout (std140) uniform shader_globals {
    //                                 aligned offset
    int num_point_lights;           // 0
    Point_Light point_lights[16];   // 16
};

uniform vec3 material_color;

//uniform vec3 light_pos;
//uniform vec3 light_color;

uniform vec3 camera_pos;

uniform sampler2D image_texture;

uniform bool use_color_override;

in vec2 uv;
in vec3 frag_pos;
in vec3 normal;

out vec4 FragColor;

vec3 calc_point_light(Point_Light point_light, vec3 material_diffuse, vec3 normal, vec3 h, vec3 l) {
    vec3 mat_spec_color = vec3(point_light.color);
    
    vec3 mat_diffuse_color = material_diffuse;

    vec3 light_spec_color = vec3(point_light.color);
    vec3 light_diffuse_color = vec3(point_light.color);

    // specular
    float gloss = 50;
    vec3 spec_contrib = light_spec_color * mat_spec_color * pow(max(dot(normal, h), 0), gloss);

    // diffuse
    vec3 diffuse_contrib = light_diffuse_color * mat_diffuse_color * max(dot(normal, l), 0);

    return spec_contrib + diffuse_contrib;
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

    vec3 mat_ambient_color = used_color;

    vec3 light_contrib = vec3(0.0);

    // ambient
    vec3 global_ambient = vec3(0.2, 0.2, 0.2);
    vec3 ambient_contrib = global_ambient * mat_ambient_color;
    light_contrib += ambient_contrib;

    #if 1
    for (int i = 0; i < num_point_lights; i++) {
        // fragment to light
        #if 1
        vec3 l = normalize(point_lights[i].position.xyz - frag_pos);
        // halfway vector
        vec3 h = normalize(v + l);

        Point_Light point_light = point_lights[i];
        light_contrib += calc_point_light(point_light, used_color, normal, h, l);
        #endif
        //light_contrib += point_lights[i].position.xyz;
    }
    #endif

    FragColor = vec4(light_contrib, 1.0);
    //FragColor = vec4(num_point_lights, light_contrib.x*0.01, 0.0, 1.0);
    //FragColor = vec4(num_point_lights, 0, 0, 1.0);
}
