#version 330 core

layout (location = 0) in vec3 pos;
//layout (location = 1) in vec3 vertex_normal;
//layout (location = 2) in vec2 vertex_uv;

//uniform mat4 cpv_matrix;

out vec3 texture_sample_dir;

uniform mat4 model_matrix;
// we separate these here because we need to modify the view matrix
uniform mat4 view_matrix;
uniform mat4 perspective_clip_matrix;

/*
cube samplers are sampled with a vector from the center of the cube

 */

void main() {
    texture_sample_dir = pos;

    // get the translation out of the view matrix.
    // keep the perspective clip as it is.
    gl_Position = perspective_clip_matrix * mat4(mat3(view_matrix)) * model_matrix * vec4(pos, 1.0);
}
