#version 330 core

uniform vec3 material_color;

uniform vec3 light_pos;
uniform vec3 light_color;

uniform vec3 camera_pos;

uniform sampler2D image_texture;

uniform bool use_color_override;

in vec2 uv;
in vec3 frag_pos;
in vec3 normal;

out vec4 FragColor;

void main() {
    // fragment to camera
    vec3 v = normalize(camera_pos - frag_pos);
    // fragment to light
    vec3 l = normalize(light_pos - frag_pos);
    // halfway vector
    vec3 h = normalize(v + l);

    vec3 mat_spec_color = light_color;
    
    vec3 used_color;
    if (use_color_override) {
        used_color = material_color;
    } else {
        used_color = texture(image_texture, uv).xyz;
    }
    vec3 mat_diffuse_color = used_color;
    vec3 mat_ambient_color = used_color;

    vec3 light_spec_color = light_color;
    vec3 light_diffuse_color = light_color;

    // specular
    float gloss = 50;
    vec3 spec_contrib = light_spec_color * mat_spec_color * pow(max(dot(normal, h), 0), gloss);

    // diffuse
    vec3 diffuse_contrib = light_diffuse_color * mat_diffuse_color * max(dot(normal, l), 0);

    // ambient
    // ambient color and strength
    vec3 global_ambient = vec3(0.2, 0.2, 0.2);
    vec3 ambient_contrib = global_ambient * mat_ambient_color;

    FragColor = vec4(spec_contrib + diffuse_contrib + ambient_contrib, 1.0);
}
